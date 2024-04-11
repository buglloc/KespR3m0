#include "httpd/server.h"

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <lwip/sockets.h>

#include "static_handler.h"
#include "ws_handler.h"
#include "ws_keep_alive.h"
#include "apps_manager.h"


namespace {
  static const char *TAG = "httpd";
  static httpd_handle_t server_ = nullptr;
  static HttpD::KeepAliveManager keepAliveManager_ = {};
  static HttpD::AppsManager appsManager_ = {};

  // esp_err_t wsKlmOpenFd(httpd_handle_t handle, int sockfd)
  // {
  //   ESP_LOGI(TAG, "new client connected. fd=%d", sockfd);
  //   esp_err_t err = HttpD::KeepAliveManager::Global(handle)->AddClient(sockfd);
  //   ESP_RETURN_ON_ERROR(err, TAG, "add client (fd=%d): %s", sockfd, esp_err_to_name(err));
  //   return ESP_OK;
  // }

  // void wsKlmCloseFd(httpd_handle_t handle, int sockfd)
  // {
  //   ESP_LOGI(TAG, "client disconnected: %d", sockfd);
  //   esp_err_t err = HttpD::KeepAliveManager::Global(handle)->RemoveClient(sockfd);
  //   if (err != ESP_OK) {
  //     ESP_LOGE(TAG, "remove client (fd=%d): %s", sockfd, esp_err_to_name(err));
  //   }
  //   close(sockfd);
  // }
}

esp_err_t HttpD::Start()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;
  config.ctrl_port = ESP_HTTPD_DEF_CTRL_PORT;
  config.server_port = CONFIG_KESPR_HTTP_PORT;
  config.max_open_sockets = CONFIG_KESPR_HTTP_MAX_CLIENTS;
  // config.open_fn = wsKlmOpenFd;
  // config.close_fn = wsKlmCloseFd;
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.global_user_ctx = &keepAliveManager_;

  ESP_LOGI(TAG, "starting HTTP Server");
  ESP_RETURN_ON_ERROR(httpd_start(&server_, &config), TAG, "start server");

  // ESP_RETURN_ON_ERROR(keepAliveManager_.Initialize(server_), TAG, "initialize keep alive manager");
  // ESP_RETURN_ON_ERROR(keepAliveManager_.Start(), TAG, "start keep alive manager");

  ESP_RETURN_ON_ERROR(HttpD::WsHandler::Register(server_, &appsManager_), TAG, "register ws handler");
  ESP_RETURN_ON_ERROR(HttpD::StaticHandler::Register(server_), TAG, "register static handler");
  return ESP_OK;
}

esp_err_t HttpD::RegisterApp(App *app)
{
  return appsManager_.Register(app);
}
