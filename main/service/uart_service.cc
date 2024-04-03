#include "uart_service.h"
#include <string>

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <defer.h>


using namespace Service;
namespace {
  static const char *TAG = "uart_svc";

  esp_err_t wsHandler(httpd_req_t *req)
  {
    if (req->method == HTTP_GET) {
      ESP_LOGI(TAG, "WS handshake done, the new connection was opened");
      return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = nullptr;
    REF_DEFER(free(buf));

    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t err = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", err);
      return err;
    }

    if (ws_pkt.len) {
      /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
      buf = static_cast<uint8_t *>(calloc(1, ws_pkt.len + 1));
      if (buf == nullptr) {
        ESP_LOGE(TAG, "Failed to calloc memory for buf");
        return ESP_ERR_NO_MEM;
      }

      ws_pkt.payload = buf;
      /* Set max_len = ws_pkt.len to get the frame payload */
      err = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", err);
        return err;
      }
    }

    // if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
    //   HttpContext *ctx = static_cast<HttpContext *>(req->user_ctx);
    //   if (ctx == nullptr || ctx->WsCb == nullptr) {
    //     ESP_LOGE(TAG, "httpd_ws try to use broken context");
    //     return ESP_FAIL;
    //   }

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
    // }

    static const char *data = R"({"error":"unsupported"})";
    ws_pkt.payload = (uint8_t*)data;;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    err = httpd_ws_send_frame(req, &ws_pkt);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", err);
    }
    return err;
  }
}

esp_err_t Uart::Start(httpd_handle_t server)
{
  httpd_uri_t wsURI = {
      .uri = "/uart/ws",
      .method = HTTP_GET,
      .handler = wsHandler,
      .user_ctx = nullptr
  };
  httpd_register_uri_handler(server, &wsURI);

  return ESP_OK;
}
