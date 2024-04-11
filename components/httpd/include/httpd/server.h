#pragma once
#include <string>
#include <esp_err.h>
#include <esp_http_server.h>

#include "app.h"


namespace HttpD
{
  esp_err_t Start();
  esp_err_t RegisterApp(App *app);
}
