#include "ws_handler.h"
#include <string>

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <defer.h>

#include "ws_keep_alive.h"
#include "ws_commander.h"


using namespace HttpD;

namespace {
  static const char *TAG = "httpd::ws";

  using BytesView = std::basic_string_view<uint8_t>;

  typedef struct {
    uint8_t scratch[CONFIG_KESPR_WS_BUF_SIZE];
    Ws::Commander *commander;
  } WsContext;

  static esp_err_t wsHandler(httpd_req_t *req)
  {
    int sockfd = httpd_req_to_sockfd(req);
    if (req->method == HTTP_GET) {
      ESP_LOGI(TAG, "ws connection established (fd=%d)", sockfd);
      return ESP_OK;
    }

    WsContext *ctx = static_cast<WsContext *>(req->user_ctx);

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    // First receive the full ws message
    /* Set max_len = 0 to get the frame len */
    esp_err_t err = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len: %s", esp_err_to_name(err));
      return err;
    }

    if (ws_pkt.len) {
      if (ws_pkt.len >= CONFIG_KESPR_WS_BUF_SIZE) {
        ESP_LOGW(TAG, "ws frame is too big (fd=%d): %d > %d", sockfd, ws_pkt.len, CONFIG_KESPR_WS_BUF_SIZE);
        return ESP_ERR_NO_MEM;
      }

      ws_pkt.payload = ctx->scratch;
      /* Set max_len = ws_pkt.len to get the frame payload */
      err = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
      if (err != ESP_OK) {
          ESP_LOGE(TAG, "httpd_ws_recv_frame failed (fd=%d): %s", sockfd, esp_err_to_name(err));
          return err;
      }
    }

    ESP_LOGD(TAG, "ws frame received (fd=%d type=%d)", sockfd, (int)ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_CONTINUE || ws_pkt.type == HTTPD_WS_TYPE_PONG) {
      return ESP_OK;
    }

    // if (ws_pkt.type == HTTPD_WS_TYPE_PONG) {
    //   // If it was a PONG, update the keep-alive
    //   return HttpD::KeepAliveManager::Global(req->handle)->TouchClient(httpd_req_to_sockfd(req));
    // }

    switch (ws_pkt.type) {
    case HTTPD_WS_TYPE_PING:
      ws_pkt.type = HTTPD_WS_TYPE_PONG;
      [[fallthrough]];
    case HTTPD_WS_TYPE_CLOSE:
      ws_pkt.len = 0;
      ws_pkt.payload = nullptr;
      return httpd_ws_send_frame(req, &ws_pkt);

    case HTTPD_WS_TYPE_TEXT: {
      BytesView reqBody{ws_pkt.payload, ws_pkt.len};
      assert(ctx->commander);
      esp_err_t err = ctx->commander->Dispatch(req, reqBody);
      ESP_RETURN_ON_ERROR(err, TAG, "dispatch failed (fd=%d): %s", sockfd, esp_err_to_name(err));
      return ESP_OK;
    }

    default:
      ESP_LOGW(TAG, "unexpected frame type (fd=%d): %d", sockfd, (int)ws_pkt.type);
      return ESP_OK;
    }
  }
};

esp_err_t WsHandler::Register(httpd_handle_t server, AppsManager *appsManager)
{
  static Ws::Commander commander{appsManager};
  static WsContext ctx = {
    .scratch = {},
    .commander = &commander
  };

  httpd_uri_t wsURI = {
    .uri = "/api/ws",
    .method = HTTP_GET,
    .handler = wsHandler,
    .user_ctx = &ctx,
    .is_websocket = true
  };

  return httpd_register_uri_handler(server, &wsURI);
}
