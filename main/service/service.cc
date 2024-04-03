#include "service.h"

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include "api_service.h"
#include "keyboard_service.h"
#include "uart_service.h"
#include "static_service.h"

namespace {
  static const char *TAG = "svc";
}

esp_err_t Service::Start()
{
  httpd_handle_t server = nullptr;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;
  config.ctrl_port = ESP_HTTPD_DEF_CTRL_PORT;
  config.server_port = CONFIG_KESPR_HTTP_PORT;
  config.uri_match_fn = httpd_uri_match_wildcard;

  ESP_LOGI(TAG, "starting HTTP Server");
  ESP_RETURN_ON_ERROR(httpd_start(&server, &config), TAG, "start server");

  ESP_LOGI(TAG, "starting api service");
  ESP_RETURN_ON_ERROR(Api::Start(server), TAG, "start api service");

#if CONFIG_KESPR_KBD_ENABLED
  ESP_LOGI(TAG, "starting keyboard service");
  ESP_RETURN_ON_ERROR(Keyboard::Start(server), TAG, "start keyboard service");
#else
  ESP_LOGI(TAG, "skip keyboard service (disabled)");
#endif

#if CONFIG_KESPR_UARTD_ENABLED
  ESP_LOGI(TAG, "starting uart service");
  ESP_RETURN_ON_ERROR(Uart::Start(server), TAG, "start uart service");
#else
  ESP_LOGI(TAG, "skip uart service (disabled)");
#endif

  // must be the last one
  ESP_LOGI(TAG, "starting static service");
  ESP_RETURN_ON_ERROR(Static::Start(server), TAG, "start static service");

  return ESP_OK;
}
