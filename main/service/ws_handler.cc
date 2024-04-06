#include "ws_handler.h"
#include <string>

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <defer.h>

#include "ws_keep_alive.h"


using namespace Service;

namespace {
  static const char *TAG = "ws_svc";

  static esp_err_t wsHandler(httpd_req_t *req)
  {
    Ws::Handler *handler = static_cast<Ws::Handler*>(req->user_ctx);
    if (handler == nullptr) {
      ESP_LOGE(TAG, "ws called with no handler provided");
      return ESP_ERR_INVALID_STATE;
    }

    if (!handler->Startred()) {
      ESP_LOGE(TAG, "service '%s' is not started yet", handler->Name().c_str());
      return ESP_ERR_NOT_FOUND;
    }

    if (req->method == HTTP_GET) {
      ESP_LOGI(TAG, "new ws connection for service '%s' was opened", handler->Name().c_str());
      return ESP_OK;
    }

    uint8_t *buf = NULL;
    REF_DEFER(free(buf));

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

    // First receive the full ws message
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
      return ret;
    }

    if (ws_pkt.len) {
      /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
      buf = static_cast<uint8_t *>(calloc(1, ws_pkt.len + 1));
      if (buf == NULL) {
        ESP_LOGE(TAG, "Failed to calloc memory for buf");
        return ESP_ERR_NO_MEM;
      }

      ws_pkt.payload = buf;
      /* Set max_len = ws_pkt.len to get the frame payload */
      ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
      if (ret != ESP_OK) {
          ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
          free(buf);
          return ret;
      }
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_PONG) {
      // If it was a PONG, update the keep-alive
      Ws::KeepAliveManager *km = static_cast<Ws::KeepAliveManager*>(httpd_get_global_user_ctx(req->handle));
      if (km == nullptr) {
        ESP_LOGE(TAG, "no keep alive manager found in global context");
        return ESP_ERR_INVALID_STATE;
      }

      return km->TouchClient(httpd_req_to_sockfd(req));
    }

    switch (ws_pkt.type) {
    case HTTPD_WS_TYPE_PING:
      ws_pkt.type = HTTPD_WS_TYPE_PONG;
      [[fallthrough]];
    case HTTPD_WS_TYPE_CLOSE:
      ws_pkt.len = 0;
      ws_pkt.payload = NULL;
      return httpd_ws_send_frame(req, &ws_pkt);

    case HTTPD_WS_TYPE_TEXT:
      ESP_LOGW(TAG, "new text msg");

    //   BytesView reqBody{ws_pkt.payload, ws_pkt.len};
    //   Bytes rspBody = ctx->WsCb(reqBody);

    //   httpd_ws_frame_t ws_pkt_rsp;
    //   memset(&ws_pkt_rsp, 0, sizeof(httpd_ws_frame_t));
    //   ws_pkt_rsp.payload = rspBody.data();
    //   ws_pkt_rsp.len = rspBody.size();
    //   ws_pkt_rsp.type = HTTPD_WS_TYPE_TEXT;

    //   err = httpd_ws_send_frame(req, &ws_pkt_rsp);
    //   if (err != ESP_OK) {
    //       ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", err);
    //   }
    //   return err;
      return ESP_OK;

    default:
      ESP_LOGW(TAG, "unexpected frame type on service '%s': %d", handler->Name().c_str(), (int)ws_pkt.type);
      return ESP_OK;
    }
  }
}

esp_err_t Ws::RegisterWsHandler(httpd_handle_t server, const HandlerConfig& cfg)
{
  httpd_uri_t wsURI = {
      .uri = cfg.uri.c_str(),
      .method = HTTP_GET,
      .handler = wsHandler,
      .user_ctx = cfg.handler,
  };
  httpd_register_uri_handler(server, &wsURI);
  return ESP_OK;
}

esp_err_t Ws::UnRegisterWsHandler(httpd_handle_t server, const HandlerConfig& cfg)
{
  httpd_unregister_uri_handler(server, cfg.uri.c_str(), HTTP_GET);
  return ESP_OK;
}
