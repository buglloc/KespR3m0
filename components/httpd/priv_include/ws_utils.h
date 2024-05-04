#pragma once
#include <functional>
#include <esp_err.h>
#include <esp_http_server.h>


namespace HttpD::Ws
{
  typedef void (*FrameBuilder)(httpd_ws_frame_t *frame, void *arg);

  esp_err_t SendAsyncFrame(httpd_handle_t server, int sockfd, FrameBuilder builder, void *arg = nullptr);
  esp_err_t SendFrame(httpd_handle_t server, int sockfd, httpd_ws_frame_t *frame);
  esp_err_t Broadcast(httpd_handle_t server, httpd_ws_frame_t *frame);
}
