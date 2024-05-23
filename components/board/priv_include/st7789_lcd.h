#pragma once
#include <esp_err.h>

#include "lovyan_gfx.h"


namespace KESPR::Display
{
  class ST7789LCD: public lgfx::LGFX_Device
  {
  public:
    ST7789LCD() = default;
    esp_err_t Initialize();

  protected:
    lgfx::Panel_ST7789 panel_;
    lgfx::Bus_SPI bus_;
    lgfx::Light_PWM backlight_;
  };
}
