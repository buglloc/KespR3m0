#include <sdkconfig.h>
#include "kespr_gui.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <esp_check.h>

#include <lvgl.h>
#include <defer.h>

#include "kespr_net.h"
#include "kespr_lvgl.h"
#include "kespr_scene.h"
#include "kespr_display.h"


using namespace KESPR::GUI;

namespace
{
  static const char* TAG = "kespr::gui";
  static SemaphoreHandle_t mu_ = nullptr;
  static KESPR::GUI::App lastApp_ = KESPR::GUI::App::None;
  static KESPR::GUI::App newApp_ = KESPR::GUI::App::None;
  static KESPR::GUI::AppState lastAppState_ = KESPR::GUI::AppState::Inactive;
  static KESPR::GUI::AppState newAppState_ = KESPR::GUI::AppState::Inactive;

  void lvTick(lv_timer_t *t)
  {
    if (xSemaphoreTake(mu_, portMAX_DELAY) == pdFALSE) {
      ESP_LOGE(TAG, "unable to get lock");
      return;
    }

    if (lastApp_ != newApp_) {
      lastApp_ = newApp_;
      Scene::SetApp(lastApp_);
    }

    if (lastAppState_ != newAppState_) {
      lastAppState_ = newAppState_;
      Scene::SetAppState(lastAppState_);
    }

    Scene::SetWiFiLevel(KESPR::Net::Signal());
    xSemaphoreGive(mu_);
  }
}

void KESPR::GUI::ChangeApp(const App app)
{
  if (xSemaphoreTake(mu_, portMAX_DELAY) == pdFALSE) {
    ESP_LOGE(TAG, "unable to get lock");
    return;
  }
  NONE_DEFER(xSemaphoreGive(mu_));

  if (lastApp_ == app) {
    return;
  }

  newApp_ = app;
  newAppState_ = AppState::Inactive;
}

void KESPR::GUI::ChangeAppState(const AppState state)
{
  if (xSemaphoreTake(mu_, portMAX_DELAY) == pdFALSE) {
    ESP_LOGE(TAG, "unable to get lock");
    return;
  }
  NONE_DEFER(xSemaphoreGive(mu_));

  if (lastAppState_ == state) {
    return;
  }

  newAppState_ = state;
}

esp_err_t KESPR::GUI::Initialize()
{
  esp_err_t err = ESP_OK;
  mu_ = xSemaphoreCreateMutex();
  if (mu_ == nullptr) {
    err = ESP_ERR_NO_MEM;
    ESP_RETURN_ON_ERROR(err, TAG, "no mutex allocated");
  }

  err = LVGL::Initialize();
  ESP_RETURN_ON_ERROR(err, TAG, "failed to initialize lvgl");

  bool locked = LVGL::Lock(0);
  ESP_RETURN_ON_FALSE(locked, ESP_FAIL, TAG, "take lvgl lock");
  NONE_DEFER(LVGL::Unlock());

  err = KESPR::GUI::Scene::Show();
  ESP_RETURN_ON_ERROR(err, TAG, "show scene");
  KESPR::Display::SetBrightness(CONFIG_KESPR_DEFAULT_BRIGHTNESS);

  ESP_LOGI(TAG, "initialize ui task timer");
  lv_timer_create(lvTick, CONFIG_KESPR_GUI_PERIOD_TIME_MS, nullptr);
  return ESP_OK;
}
