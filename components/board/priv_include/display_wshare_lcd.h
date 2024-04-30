#pragma once
#include <esp_err.h>

#include <LovyanGFX.hpp>


namespace KESPR::Display
{
  class WShareLCD: public lgfx::LGFX_Device
  {
  public:
    WShareLCD() = default;
    esp_err_t Initialize();

  protected:
    lgfx::Panel_GC9A01 panel_;
    lgfx::Bus_SPI bus_;
    lgfx::Light_PWM backlight_;
  };
}
