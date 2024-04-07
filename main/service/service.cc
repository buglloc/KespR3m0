#include "service.h"

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <lwip/sockets.h>

#include "ws_keep_alive.h"
#include "api_service.h"
#include "usbkb_service.h"
#include "uart_service.h"
#include "static_service.h"


namespace {
  static const char *TAG = "svc";
  const uint16_t kMaxUriHandlers = 8;
  static httpd_handle_t server_ = nullptr;
  static Service::StaticService staticService_;
  static Service::ApiService apiService_;
  static Service::UsbkbService usbkbService_;
  static Service::UartService uartService_;
  static Service::Ws::KeepAliveManager keepAliveManager_;

  esp_err_t wsKlmOpenFd(httpd_handle_t server, int sockfd)
  {
    (void)server;
    ESP_LOGD(TAG, "new client connected. fd=%d", sockfd);
    return keepAliveManager_.AddClient(sockfd);
  }

  void wsKlmCloseFd(httpd_handle_t server, int sockfd)
  {
    (void)server;
    ESP_LOGD(TAG, "client disconnected: %d", sockfd);
    esp_err_t err = keepAliveManager_.RemoveClient(sockfd);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "unable to remove client (fd=%d): %s", sockfd, esp_err_to_name(err));
    }
    close(sockfd);
  }
}

esp_err_t Service::Start()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;
  config.ctrl_port = ESP_HTTPD_DEF_CTRL_PORT;
  config.server_port = CONFIG_KESPR_HTTP_PORT;
  config.max_uri_handlers = kMaxUriHandlers;
  config.max_open_sockets = CONFIG_KESPR_HTTP_MAX_CLIENTS;
  config.open_fn = wsKlmOpenFd;
  config.close_fn = wsKlmCloseFd;
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.global_user_ctx = &keepAliveManager_;

  ESP_LOGI(TAG, "starting HTTP Server");
  ESP_RETURN_ON_ERROR(httpd_start(&server_, &config), TAG, "start server");

  ESP_RETURN_ON_ERROR(keepAliveManager_.Initialize(server_), TAG, "initialize keep alive manager");
  ESP_RETURN_ON_ERROR(keepAliveManager_.Start(), TAG, "start keep alive manager");

  ESP_RETURN_ON_ERROR(apiService_.Initialize(server_), TAG, "initialize api service");
  ESP_RETURN_ON_ERROR(apiService_.Start(), TAG, "start api service");

#if CONFIG_KESPR_USBKBD_ENABLED
  ESP_RETURN_ON_ERROR(usbkbService_.Initialize(server_), TAG, "initialize usbkb service");
  ESP_RETURN_ON_ERROR(apiService_.RegisterService(&usbkbService_), TAG, "register usbkb service");
#else
  ESP_LOGI(TAG, "skip usbkb service (disabled)");
#endif

#if CONFIG_KESPR_UARTD_ENABLED
  ESP_RETURN_ON_ERROR(uartService_.Initialize(server_), TAG, "initialize uart service");
  ESP_RETURN_ON_ERROR(apiService_.RegisterService(&uartService_), TAG, "register uart service");
#else
  ESP_LOGI(TAG, "skip uart service (disabled)");
#endif

  ESP_RETURN_ON_ERROR(staticService_.Initialize(server_), TAG, "initialize static service");
  ESP_RETURN_ON_ERROR(staticService_.Start(), TAG, "start static service");
  return ESP_OK;
}
