#include "ws_base_service.h"
#include <string>
#include <esp_err.h>
#include <esp_check.h>
#include <esp_http_server.h>

#include "ws_utils.h"


using namespace Service::Ws;
namespace {
  static const char *TAG = "ws_svc";
}

esp_err_t BaseService::Broadcast(FrameBuilder builder, void *arg)
{
  if (!this->httpd_) {
    return ESP_ERR_INVALID_STATE;
  }

  size_t clients = CONFIG_KESPR_HTTP_MAX_CLIENTS;
  int    client_fds[CONFIG_KESPR_HTTP_MAX_CLIENTS];
  esp_err_t err = httpd_get_client_list(this->httpd_, &clients, client_fds);
  ESP_RETURN_ON_ERROR(err, TAG, "httpd_get_client_list failed");

  for (size_t i = 0; i < clients; ++i) {
    int sock = client_fds[i];
    if (httpd_ws_get_fd_info(this->httpd_, sock) != HTTPD_WS_CLIENT_WEBSOCKET) {
      continue;
    }

    AsyncFrame *frame = new AsyncFrame({
      .server = this->httpd_,
      .fd = sock,
      .builder = builder,
      .builderArg = arg
    });
    esp_err_t err = SendAsyncFrame(frame);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "httpd_queue_work failed");
      continue;
    }
  }

  return ESP_OK;
}
