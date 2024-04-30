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

  display_.setBrightness(CONFIG_KESPR_DISPLAY_BRIGHTNESS);
  display_.setTextSize((std::max(display_.width(), display_.height()) + 255) >> 8);
  display_.fillScreen(TFT_WHITE);

  uint32_t count = ~0;
  {
    display_.startWrite();
    display_.setRotation(++count & 7);

    display_.setTextColor(TFT_BLACK);
    display_.drawRect(60,60,display_.width()-120,display_.height()-120,count*7);
    display_.drawFastHLine(0, 0, 10);

    display_.endWrite();
  }

  return ESP_OK;
}
