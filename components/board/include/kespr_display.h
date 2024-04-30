#pragma once
#include <esp_err.h>


namespace KESPR::Display
{
  esp_err_t Initialize();

  void PushPixels(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *data);

  uint8_t GetBrightness();
  void SetBrightness(uint8_t brightness);
  uint32_t Width();
  uint32_t Height();
}
