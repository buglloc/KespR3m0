#include "kespr_net.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_check.h>
#include <esp_mac.h>

#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/ip_addr.h>
#include <lwip/inet.h>

#include <dns_server.h>


using namespace KESPR;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

namespace
{
  const char* TAG = "kespr::net";
  EventGroupHandle_t stateGroup_;
  int retryNum_ = 0;
  bool started_ = false;
  Net::Mode mode_ = Net::Mode::None;
  esp_ip4_addr_t ipAddr_ = {
    .addr = IPADDR_NONE
  };

#if CONFIG_KESPR_NET_STA_USE_STATIC_IP
  esp_err_t setDNSServer(esp_netif_t *netif, uint32_t addr, esp_netif_dns_type_t type)
  {
    if (addr && (addr != IPADDR_NONE)) {
      esp_netif_dns_info_t dns;
      dns.ip.u_addr.ip4.addr = addr;
      dns.ip.type = IPADDR_TYPE_V4;
      ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, type, &dns));
    }
    return ESP_OK;
  }

  void setIPSTA(esp_netif_t *netif)
  {
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
      ESP_LOGE(TAG, "failed to stop dhcp client");
      return;
    }

    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(CONFIG_KESPR_NET_STA_IP);
    ip.netmask.addr = ipaddr_addr(CONFIG_KESPR_NET_STA_SUBNET);
    ip.gw.addr = ipaddr_addr(CONFIG_KESPR_NET_STA_GW);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
      ESP_LOGE(TAG, "failed to set ip info");
      return;
    }

    ESP_LOGI(TAG, "success to set static ip: %s, netmask: %s, gw: %s", CONFIG_KESPR_NET_STA_IP, CONFIG_KESPR_NET_STA_SUBNET, CONFIG_KESPR_NET_STA_GW);
    ESP_ERROR_CHECK(setDNSServer(netif, ipaddr_addr(CONFIG_KESPR_NET_STA_DNS), ESP_NETIF_DNS_MAIN));
  }
#else
  void setIPSTA(esp_netif_t *netif)
  {
    (void)(netif);
  }
#endif

  void ipEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
  {
    if (event_id != IP_EVENT_STA_GOT_IP) {
      return;
    }

    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    memcpy(&ipAddr_, &event->ip_info.ip, sizeof(esp_ip4_addr_t));

    retryNum_ = 0;
    xEventGroupSetBits(stateGroup_, WIFI_CONNECTED_BIT);
  }

  void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
  {
    switch (event_id) {
    case WIFI_EVENT_STA_START: {
      esp_wifi_connect();
      break;
    }

    case WIFI_EVENT_STA_CONNECTED: {
      setIPSTA(static_cast<esp_netif_t *>(arg));
      break;
    }

    case WIFI_EVENT_STA_DISCONNECTED: {
      ipAddr_.addr = IPADDR_NONE;

      wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t *)event_data;
      ESP_LOGE(TAG, "wifi disconnected, reason: %d", disconnected->reason);

      if (retryNum_ >= CONFIG_KESPR_NET_CONNECT_RETRIES) {
        xEventGroupSetBits(stateGroup_, WIFI_FAIL_BIT);
        return;
      }

      esp_wifi_connect();
      retryNum_++;
      ESP_LOGI(TAG, "retry to connect to the AP '%s'. Try num: %d", CONFIG_KESPR_NET_STA_SSID, retryNum_);
      break;
    }

    case WIFI_EVENT_AP_STACONNECTED: {
      wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
      ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
      break;
    }

    case WIFI_EVENT_AP_STADISCONNECTED: {
      wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
      ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
      break;
    }

    default:
      break;
    }
  }

  esp_err_t connectSTA()
  {
    xEventGroupClearBits(stateGroup_, WIFI_CONNECTED_BIT);
    xEventGroupClearBits(stateGroup_, WIFI_FAIL_BIT);

    esp_netif_t *staNetif = esp_netif_create_default_wifi_sta();
    assert(staNetif);
    ESP_RETURN_ON_ERROR(esp_netif_set_hostname(staNetif, CONFIG_KESPR_HOSTNAME), TAG, "set hostname");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "initialize wifi");

    esp_err_t err = esp_event_handler_instance_register(
      WIFI_EVENT,
      ESP_EVENT_ANY_ID,
      &wifiEventHandler,
      staNetif,
      nullptr
    );
    ESP_RETURN_ON_ERROR(err, TAG, "register wifi event handler");

    err = esp_event_handler_instance_register(
      IP_EVENT,
      IP_EVENT_STA_GOT_IP,
      &ipEventHandler,
      staNetif,
      nullptr
    );
    ESP_RETURN_ON_ERROR(err, TAG, "register ip event handler");

    wifi_config_t wifi_config = {
      .sta = {
        .ssid = CONFIG_KESPR_NET_STA_SSID,
        .password = CONFIG_KESPR_NET_STA_PASSWORD,
        .threshold = {
          .authmode = WIFI_AUTH_WPA2_PSK
        },
      },
    };
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "set wifi mode");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG, "conrigure wifi");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "wifi start");

    ESP_LOGI(TAG, "wait STA connections...");
    EventBits_t bits = xEventGroupWaitBits(
      stateGroup_,
      WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
      pdFALSE,
      pdFALSE,
      portMAX_DELAY
    );

    if (bits & WIFI_CONNECTED_BIT) {
      mode_ = Net::Mode::STA;
      ESP_LOGI(TAG, "connected to ap SSID: %s", CONFIG_KESPR_NET_STA_SSID);
      return ESP_OK;
    }

    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &ipEventHandler);
    esp_wifi_stop();
    esp_netif_destroy_default_wifi(staNetif);
    if (bits & WIFI_FAIL_BIT) {
      ESP_LOGI(TAG, "Failed to connect to SSID: %s", CONFIG_KESPR_NET_STA_SSID);
      return ESP_FAIL;
    }

    ESP_LOGE(TAG, "UNEXPECTED EVENT");
    return ESP_FAIL;
  }

  esp_err_t connectAP()
  {
    esp_netif_t *staNetif = esp_netif_create_default_wifi_ap();
    assert(staNetif);
    ESP_RETURN_ON_ERROR(esp_netif_set_hostname(staNetif, CONFIG_KESPR_HOSTNAME), TAG, "set hostname");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "initialize wifi");

    esp_err_t err = esp_event_handler_instance_register(
      WIFI_EVENT,
      ESP_EVENT_ANY_ID,
      &wifiEventHandler,
      staNetif,
      nullptr
    );
    ESP_RETURN_ON_ERROR(err, TAG, "register wifi event handler");

    wifi_config_t wifi_config = {
      .ap = {
        .ssid = CONFIG_KESPR_NET_AP_SSID,
        .password = CONFIG_KESPR_NET_AP_PASSWORD,
        .ssid_len = strlen(CONFIG_KESPR_NET_AP_SSID),
        .channel = CONFIG_KESPR_NET_AP_CHANNEL,
        .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        .max_connection = CONFIG_KESPR_NET_AP_CONN,
        .pmf_cfg = {
          .required = true,
        },
      },
    };
    if (strlen(CONFIG_KESPR_NET_AP_PASSWORD) == 0) {
      wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_AP), TAG, "set wifi mode");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_AP, &wifi_config), TAG, "conrigure wifi");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "wifi start");

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

    ipAddr_ = ip_info.ip;
    mode_ = Net::Mode::AP;
    ESP_LOGI(TAG, "wifi AP started. SSID:%s IP:" IPSTR " Channel:%d", CONFIG_KESPR_NET_AP_SSID,  IP2STR(&ipAddr_), CONFIG_KESPR_NET_AP_CHANNEL);

    // Start the DNS server that will redirect all queries to the softAP IP
    dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE("*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
    start_dns_server(&config);
    return ESP_OK;
  }
}

esp_err_t Net::Start()
{
  if (started_) {
    ESP_LOGW(TAG, "network already initialized");
    return ESP_OK;
  }

  stateGroup_ = xEventGroupCreate();
  esp_err_t err = esp_netif_init();
  ESP_RETURN_ON_ERROR(err, TAG, "initialize netif");

  ESP_LOGI(TAG, "connect STA");
  err = connectSTA();
  if (err == ESP_OK) {
    return ESP_OK;
  }

  ESP_LOGW(TAG, "STA connection failed, switch to AP");
  return connectAP();
}

esp_ip4_addr_t Net::CurrentIP() {
  return ipAddr_;
}

Net::Mode Net::CurrentMode() {
  return mode_;
}

int8_t Net::Signal() {
  if (mode_ == Net::Mode::None) {
    return 0;
  }

  if (mode_ == Net::Mode::AP) {
    return 100;
  }

  wifi_ap_record_t ap;
  esp_err_t err = esp_wifi_sta_get_ap_info(&ap);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "unable to get ap info: %s", esp_err_to_name(err));
    return 0;
  }

  if (ap.rssi < -90 || ap.rssi >= 0) {
    return 0;
  }

  return 100 + ap.rssi / 2;
}
