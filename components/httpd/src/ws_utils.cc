#include "ws_utils.h"
#include <esp_check.h>


using namespace HttpD;
namespace {
  static const char *TAG = "httpd::ws::utils";

  typedef struct {
    httpd_handle_t server = nullptr;
    int sockfd = 0;
    Ws::FrameBuilder builder = nullptr;
    void *builderArg = nullptr;
  } AsyncFrame;

  void sendAsync(void *argsPtr)
  {
    AsyncFrame *args = static_cast<AsyncFrame *>(argsPtr);
    if (args->builder == nullptr) {
      ESP_LOGW(TAG, "no builder provided -> nothing to send");
      return;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    (*args->builder)(&ws_pkt, args->builderArg);
    httpd_ws_send_frame_async(args->server, args->sockfd, &ws_pkt);
    delete(args);
  }
}

esp_err_t Ws::SendAsyncFrame(httpd_handle_t server, int sockfd, FrameBuilder builder, void *arg)
{
  if (!server || !builder) {
    return ESP_ERR_INVALID_ARG;
  }

  AsyncFrame *frame = new AsyncFrame({
    .server = server,
    .sockfd = sockfd,
    .builder = builder,
    .builderArg = arg
  });
  return httpd_queue_work(server, sendAsync, (void *)frame);
}

esp_err_t Ws::SendFrame(httpd_handle_t server, int sockfd, httpd_ws_frame_t *frame)
{
  if (!server || !frame) {
    return ESP_ERR_INVALID_STATE;
  }

  return httpd_ws_send_frame_async(server, sockfd, frame);
}

esp_err_t Ws::Broadcast(httpd_handle_t server, httpd_ws_frame_t *frame)
{
  if (!server || !frame) {
    return ESP_ERR_INVALID_STATE;
  }

  size_t clients = CONFIG_KESPR_HTTP_MAX_CLIENTS;
  int    client_fds[CONFIG_KESPR_HTTP_MAX_CLIENTS];
  esp_err_t err = httpd_get_client_list(server, &clients, client_fds);
  ESP_RETURN_ON_ERROR(err, TAG, "httpd_get_client_list failed");

  for (size_t i = 0; i < clients; ++i) {
    int sock = client_fds[i];
    if (sock == 0) {
      continue;
    }

    if (httpd_ws_get_fd_info(server, sock) != HTTPD_WS_CLIENT_WEBSOCKET) {
      continue;
    }

    esp_err_t err = httpd_ws_send_frame_async(server, sock, frame);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "httpd_queue_work failed: %s", esp_err_to_name(err));
      continue;
    }
  }

  return ESP_OK;
}
