#include "kespr_gui.h"

#include <stdlib.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <esp_check.h>

#include <lvgl.h>
#include <defer.h>

#include <appsman/manager.h>

#include "kespr_lvgl.h"
#include "kespr_scene.h"
#include "kespr_display.h"
#include "kespr_net.h"
#include "kespr_power.h"


using namespace KESPR::GUI;

namespace
{
  const char* TAG = "kespr::gui";
  constexpr int8_t kLevelThreshold = 5;

  App lastApp_ = App::None;
  AppState lastAppState_ = AppState::Inactive;
  int8_t lastWiFiSignal_ = 0;
  int8_t lastBattLevel_ = 0;
  uint16_t ticksToUpdateInfo_ = 0;

  void lvTick(lv_timer_t *t)
  {
    App curApp = static_cast<App>(AppsMan::Manager::CurrentApp()->ID());
    if (lastApp_ != curApp) {
      lastApp_ = curApp;
      Scene::SetApp(lastApp_);
    }

    // if (lastAppState_ != newAppState_) {
    //   lastAppState_ = newAppState_;
    //   Scene::SetAppState(lastAppState_);
    // }

    if (ticksToUpdateInfo_ > 0) {
      ticksToUpdateInfo_--;
      return;
    }
    ticksToUpdateInfo_ = (CONFIG_KESPR_GUI_INFO_UPDATE_PERIOD_MS / CONFIG_KESPR_GUI_PERIOD_TIME_MS);

    uint8_t curWifiSignal = KESPR::Net::Signal();
    if (abs(lastWiFiSignal_ - curWifiSignal) > kLevelThreshold) {
      lastWiFiSignal_ = curWifiSignal;
      Scene::SetWiFiLevel(lastWiFiSignal_);
    }

    uint8_t curBattLevel = KESPR::Power::BattLevel();
    if (abs(lastBattLevel_ - curBattLevel) > kLevelThreshold) {
      lastBattLevel_ = curBattLevel;
      Scene::SetBattLevel(lastBattLevel_);
    }
  }
}

esp_err_t KESPR::GUI::Initialize()
{
  esp_err_t err = ESP_OK;

  err = LVGL::Initialize();
  ESP_RETURN_ON_ERROR(err, TAG, "failed to initialize lvgl");

  bool locked = LVGL::Lock(0);
  ESP_RETURN_ON_FALSE(locked, ESP_FAIL, TAG, "take lvgl lock");
  DEFER{ LVGL::Unlock(); };

  err = KESPR::GUI::Scene::Show();
  ESP_RETURN_ON_ERROR(err, TAG, "show scene");
  KESPR::Display::SetBrightness(CONFIG_KESPR_DEFAULT_BRIGHTNESS);

  ESP_LOGI(TAG, "initialize ui task timer");
  lv_timer_create(lvTick, CONFIG_KESPR_GUI_PERIOD_TIME_MS, nullptr);
  return ESP_OK;
}
