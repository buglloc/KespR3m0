#pragma once
#include <esp_err.h>


namespace KESPR::Power
{
  esp_err_t Initialize();
  int8_t BattLevel();
}
