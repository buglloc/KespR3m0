#pragma once
#include <esp_err.h>
#include <esp_http_server.h>


namespace Service::Uart
{
  esp_err_t Start(httpd_handle_t server);
}
