#include "httpd/server.h"

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <lwip/sockets.h>

#include "static_handler.h"
#include "ws_handler.h"
#include "ws_utils.h"


namespace {
  const char *TAG = "httpd";
  httpd_handle_t server_ = nullptr;
}

esp_err_t HttpD::Start(const WsMsgHandler& msgHandler)
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;
  config.keep_alive_enable = true;
  config.enable_so_linger = true;
  config.ctrl_port = ESP_HTTPD_DEF_CTRL_PORT;
  config.server_port = CONFIG_KESPR_HTTP_PORT;
  config.max_open_sockets = CONFIG_KESPR_HTTP_MAX_CLIENTS;
  config.uri_match_fn = httpd_uri_match_wildcard;

  ESP_LOGI(TAG, "starting HTTP Server");
  ESP_RETURN_ON_ERROR(httpd_start(&server_, &config), TAG, "start server");

  ESP_RETURN_ON_ERROR(HttpD::WsHandler::Register(server_, msgHandler), TAG, "register ws handler");
  ESP_RETURN_ON_ERROR(HttpD::StaticHandler::Register(server_), TAG, "register static handler");
  return ESP_OK;
}

esp_err_t HttpD::BroadcastMsg(const JsonObjectConst& msg)
{
  std::string out;
  if (serializeJson(msg, out) == 0) {
    ESP_LOGE(TAG, "unable to serialize msg");
    return ESP_FAIL;
  }

  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.payload = reinterpret_cast<uint8_t *>(out.data());
  ws_pkt.len = out.size();
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  return Ws::Broadcast(server_, &ws_pkt);
}

esp_err_t HttpD::SendMsg(int sockfd, const JsonObjectConst& msg)
{
  std::string out;
  if (serializeJson(msg, out) == 0) {
    ESP_LOGE(TAG, "unable to serialize msg");
    return ESP_FAIL;
  }

  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.payload = reinterpret_cast<uint8_t *>(out.data());
  ws_pkt.len = out.size();
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  return Ws::SendFrame(server_, sockfd, &ws_pkt);
}
