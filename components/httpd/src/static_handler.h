#pragma once
#include <esp_err.h>
#include <esp_http_server.h>


namespace HttpD::StaticHandler
{
  esp_err_t Register(httpd_handle_t server);
}
