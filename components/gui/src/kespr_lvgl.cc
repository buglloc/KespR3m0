#include "kespr_lvgl.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <esp_err.h>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_sleep.h>

#include <lvgl.h>

#include <kespr_display.h>


using namespace KESPR::GUI;

namespace
{
  const char* TAG = "kespr::gui::lvgl";
  constexpr uint32_t kMaxSleepMs = CONFIG_KESPR_GUI_PERIOD_TIME_MS * 2;
  SemaphoreHandle_t guiMu_ = {};
  lv_display_t *lvDisp_ = nullptr;

  void lvTask(void *arg)
  {
    ESP_LOGI(TAG, "starting LVGL task");
    LV_UNUSED(arg);
    uint32_t taskDelayMS = 0;
    for (;;)
    {
      taskDelayMS = kMaxSleepMs;
      if (LVGL::Lock(0)) {
        taskDelayMS = lv_task_handler();
        LVGL::Unlock();
      }

      if (taskDelayMS > kMaxSleepMs || taskDelayMS == 1) {
        taskDelayMS = kMaxSleepMs;
      } else if (taskDelayMS < 1) {
        taskDelayMS = 1;
      }

      vTaskDelay(pdMS_TO_TICKS(taskDelayMS));
    }
  }

  void lvTick(void *arg)
  {
    LV_UNUSED(arg);
    lv_tick_inc(CONFIG_KESPR_GUI_LVGL_TICK_MS);
  }

 #if LV_USE_LOG
  void lvLog(lv_log_level_t level, const char *buf)
  {
    switch (level) {
      case LV_LOG_LEVEL_TRACE:
        ESP_LOGD(TAG, "%s", buf);
        return;

      case LV_LOG_LEVEL_INFO:
        ESP_LOGI(TAG, "%s", buf);
        return;

      case LV_LOG_LEVEL_WARN:
        ESP_LOGW(TAG, "%s", buf);
        return;

      case LV_LOG_LEVEL_ERROR:
        ESP_LOGE(TAG, "%s", buf);
        return;

      case LV_LOG_LEVEL_USER:
        ESP_LOGI(TAG, "%s", buf);
        return;

      case LV_LOG_LEVEL_NONE:
        return;

      default:
        ESP_LOGE(TAG, "unexpected[%d]: %s", static_cast<int>(level), buf);
        return;
    };
  }
#endif
}

esp_err_t LVGL::Initialize()
{
  esp_err_t err;
  guiMu_ = xSemaphoreCreateMutex();
  if (guiMu_ == nullptr) {
    err = ESP_ERR_NO_MEM;
    ESP_RETURN_ON_ERROR(err, TAG, "no mutex allocated");
  }

  lv_init();

  #if LV_USE_LOG
    lv_log_register_print_cb(lvLog);
  #endif

  // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
  uint32_t bufSize = KESPR::Display::Width() * KESPR::Display::Height() * sizeof(lv_color16_t) / 10;
  ESP_LOGI(TAG, "%lu - %lu - %lu - %d", bufSize, KESPR::Display::Width(), KESPR::Display::Height(), sizeof(lv_color16_t));
  lv_color16_t *buf1 = reinterpret_cast<lv_color16_t *>(heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  assert(buf1);
  lv_color16_t *buf2 = reinterpret_cast<lv_color16_t *>(heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  assert(buf2);

  lvDisp_ = lv_display_create(KESPR::Display::Width(), KESPR::Display::Height());
  assert(lvDisp_);

  lv_display_set_buffers(lvDisp_, buf1, buf2, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(lvDisp_, [](lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) -> void {
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    KESPR::Display::PushPixels(area->x1, area->y1, w, h, px_map);
    lv_disp_flush_ready(disp);
  });

  /* Create and start a periodic timer interrupt to call lv_tick_inc */
  const esp_timer_create_args_t periodic_timer_args = {
    .callback = lvTick,
    .name = "lvgl_tick",
  };
  esp_timer_handle_t periodic_timer;
  err = esp_timer_create(&periodic_timer_args, &periodic_timer);
  ESP_RETURN_ON_ERROR(err, TAG, "create lvgl_tick timer");

  err = esp_timer_start_periodic(periodic_timer, CONFIG_KESPR_GUI_LVGL_TICK_MS * 1000);
  ESP_RETURN_ON_ERROR(err, TAG, "start lvgl_tick timer");

  BaseType_t task;
#if CONFIG_KESPR_GUI_LVGL_TASK_AFFINITY >= 0
  task = xTaskCreatePinnedToCore(
    lvTask,
    "lvgl_task",
    CONFIG_KESPR_GUI_LVGL_TASK_STACK,
    nullptr,
    tskIDLE_PRIORITY+CONFIG_KESPR_GUI_LVGL_TASK_PRIO,
    nullptr,
    CONFIG_KESPR_GUI_LVGL_TASK_AFFINITY
  );
#else
  task = xTaskCreate(
    lvTask,
    "lvgl_task",
    CONFIG_KESPR_GUI_LVGL_TASK_STACK * 2,
    nullptr,
    tskIDLE_PRIORITY+CONFIG_KESPR_GUI_LVGL_TASK_PRIO,
    nullptr
  );
#endif

  ESP_RETURN_ON_FALSE(task == pdPASS, ESP_FAIL, TAG, "create LVGL task fail");
  return ESP_OK;
}

bool LVGL::Lock(uint32_t timeoutMs)
{
    assert(guiMu_ && "LVGL::Initialize must be called first");

    const TickType_t timeout = (timeoutMs == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs);
    return xSemaphoreTake(guiMu_, timeout) == pdTRUE;
}

void LVGL::Unlock()
{
    assert(guiMu_ && "LVGL::Initialize must be called first");
    xSemaphoreGive(guiMu_);
}
