#pragma once

#include <esp_err.h>

namespace KESPR::GUI::LVGL
{
  esp_err_t Initialize();

  bool Lock(uint32_t timeoutMs);
  void Unlock();
}
