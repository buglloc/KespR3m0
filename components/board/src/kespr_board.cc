#include "kespr_board.h"

#include <esp_check.h>
#include "esp_event.h"
#include <esp_log.h>

#include <nvs_flash.h>


using namespace KESPR;

namespace
{
  static const char* TAG = "kespr::board";
  static bool initialized_ = false;
}

esp_err_t Board::Initialize()
{
  if (initialized_) {
    ESP_LOGW(TAG, "board already initialized");
    return ESP_OK;
  }

  ESP_LOGI(TAG, "initialize nvs");
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, "erase nvs");
    err = nvs_flash_erase();
    ESP_RETURN_ON_ERROR(err, TAG, "nvs erasing failed");
    err = nvs_flash_init();
  }
  ESP_RETURN_ON_ERROR(err, TAG, "failed to initialize nvs");

  err = esp_event_loop_create_default();
  ESP_RETURN_ON_ERROR(err, TAG, "failed to initialize ESP event loop");

  initialized_ = true;
  return ESP_OK;
}
