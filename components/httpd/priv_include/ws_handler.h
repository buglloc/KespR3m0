#pragma once
#include <string>

#include <esp_err.h>
#include <esp_http_server.h>

#include "httpd/server.h"

namespace HttpD::WsHandler
{
  esp_err_t Register(httpd_handle_t server, const WsMsgHandler& msgHandler);
}
