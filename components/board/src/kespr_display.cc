#include "kespr_display.h"

#include <esp_check.h>
#include <esp_event.h>
#include <esp_log.h>

#include "display_wshare_lcd.h"


using namespace KESPR;

namespace
{
  static const char* TAG = "kespr::display";
  static bool initialized_ = false;
  static Display::WShareLCD display_ = {}; 
}

esp_err_t Display::Initialize()
{
  if (initialized_) {
    ESP_LOGW(TAG, "display already initialized");
    return ESP_OK;
  }

  esp_err_t err = display_.Initialize();
  ESP_RETURN_ON_ERROR(err, TAG, "initialize waveshare display");

  initialized_ = true;

  display_.setColorDepth(16);
  display_.fillScreen(TFT_BLACK);
  return ESP_OK;
}

void Display::PushPixels(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *data)
{
  display_.startWrite();
  display_.setAddrWindow(x, y, w, h);
  display_.pushPixels(data, w * h, true);
  display_.endWrite();
}

uint8_t Display::GetBrightness()
{
  return display_.getBrightness();
}

void Display::SetBrightness(uint8_t brightness)
{
  return display_.setBrightness(brightness);
}

uint32_t Display::Width()
{
  return display_.width();
}

uint32_t Display::Height()
{
  return display_.height();
}
