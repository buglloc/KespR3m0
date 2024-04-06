#pragma once
#include <functional>
#include <esp_err.h>
#include <esp_http_server.h>


namespace Service::Ws
{
  typedef void (*FrameBuilder)(httpd_ws_frame_t *frame, void *arg);

  typedef struct {
    httpd_handle_t server = nullptr;
    int fd = 0;
    FrameBuilder builder = nullptr;
    void *builderArg = nullptr;
  } AsyncFrame;

  esp_err_t SendAsyncFrame(const AsyncFrame *frame);
}
