#include "ws_utils.h"
#include <esp_check.h>


using namespace Service::Ws;
namespace {
  static const char *TAG = "ws_utl";

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
    httpd_ws_send_frame_async(args->server, args->fd, &ws_pkt);
    delete(args);
  }
}

esp_err_t Service::Ws::SendAsyncFrame(const AsyncFrame *frame)
{
  if (!frame || !frame->server) {
    return ESP_ERR_INVALID_ARG;
  }

  return httpd_queue_work(frame->server, sendAsync, (void *)frame);
}
