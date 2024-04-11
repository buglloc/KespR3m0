#include "httpd/events.h"
#include <esp_log.h>

#include "ws_utils.h"


using namespace HttpD;
namespace {
  static const char *TAG = "httpd::events";
}

esp_err_t HttpD::BroadcastEvent(httpd_handle_t server, const JsonObjectConst& event)
{
  std::string out;
  if (serializeJson(event, out) == 0) {
    ESP_LOGE(TAG, "unable to serialize event");
    return ESP_FAIL;
  }

  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.payload = (uint8_t*)out.data();
  ws_pkt.len = out.size();
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  return Ws::Broadcast(server, &ws_pkt);
}

esp_err_t HttpD::SendEvent(httpd_handle_t server, int fd, const JsonObjectConst& event)
{
  std::string out;
  if (serializeJson(event, out) == 0) {
    ESP_LOGE(TAG, "unable to serialize event");
    return ESP_FAIL;
  }

  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.payload = (uint8_t*)out.data();
  ws_pkt.len = out.size();
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  return Ws::SendFrame(server, fd, &ws_pkt);
}
