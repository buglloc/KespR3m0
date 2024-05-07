#include "kespr_power.h"
#include <driver/gpio.h>

#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>

#include <esp_adc/adc_oneshot.h>


using namespace KESPR;
namespace
{
  const char* TAG = "kespr::power";
  constexpr int kMinLevel = 1200;
  constexpr int kMaxLevel = 1500;

  bool initialized_ = false;
  adc_unit_t adcUnit_ = ADC_UNIT_1;
  adc_channel_t adcChannel_ = ADC_CHANNEL_0;
  adc_oneshot_unit_handle_t adcHandle_ = nullptr;

  int meanValue()
  {
    static int acc = 0;
    static int cnt = 0;
    static int last = 0;

    int cur = 0;
    esp_err_t err = adc_oneshot_read(adcHandle_, adcChannel_, &cur);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "adc read fail: %s", esp_err_to_name(err));
      return last;
    }

    acc += cur;
    cnt++;
    if (cnt > 3) {
      last = acc/cnt;
      cnt = 0;
      acc = 0;
    }

    return last;
  }
}

esp_err_t Power::Initialize()
{
  if (initialized_) {
    ESP_LOGW(TAG, "power manager already initialized");
    return ESP_OK;
  }

  ESP_LOGI(TAG, "initialize power manager");
  esp_err_t err = adc_oneshot_io_to_channel(CONFIG_KESPR_BATT_IO_NUM, &adcUnit_, &adcChannel_);
  ESP_RETURN_ON_ERROR(err, TAG, "find channel by GPIO #%d", CONFIG_KESPR_BATT_IO_NUM);
  ESP_LOGI(TAG, "channel found: GPIO#%d = %d (unit) / %d (channel)", CONFIG_KESPR_BATT_IO_NUM, static_cast<int>(adcUnit_), static_cast<int>(adcChannel_));

  adc_oneshot_unit_init_cfg_t initCfg = {
      .unit_id = adcUnit_,
  };
  err = adc_oneshot_new_unit(&initCfg, &adcHandle_);
  ESP_RETURN_ON_ERROR(err, TAG, "create one shot unit");

  adc_oneshot_chan_cfg_t chanCfg = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12,
  };
  err = adc_oneshot_config_channel(adcHandle_, adcChannel_, &chanCfg);
  ESP_RETURN_ON_ERROR(err, TAG, "configure channel");

  initialized_ = true;
  return ESP_OK;
}

int8_t Power::BattLevel() {
  int val = meanValue();
  if (val >= kMaxLevel) {
    return 100;
  }

  val -= kMinLevel;
  if (val <= 0) {
    return 0;
  }

  return (val * 100) / (kMaxLevel - kMinLevel);
}
