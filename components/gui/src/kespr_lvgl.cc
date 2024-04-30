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
  static const char* TAG = "kespr::gui::lvgl";
  static SemaphoreHandle_t guiMu_ = {};
  static lv_display_t *lvDisp_ = nullptr;

  void lvGuiTask(void *arg)
  {
    ESP_LOGI(TAG, "starting LVGL task");
    uint32_t taskDelayMS = 0;
    for (;;)
    {
      taskDelayMS = 0;
      if (xSemaphoreTake(guiMu_, portMAX_DELAY) == pdTRUE) {
        taskDelayMS = lv_task_handler();
        xSemaphoreGive(guiMu_);
      }

      if (taskDelayMS > 500) {
        taskDelayMS = 500;
      } else if (taskDelayMS < 5) {
        taskDelayMS = 5;
      }

      vTaskDelay(pdMS_TO_TICKS(taskDelayMS));
    }
  }
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
    lv_log_register_print_cb([](const char* buf) -> void {
      ESP_LOGI(LV_TAG, "%s", buf);
    });
  #endif

  // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
  uint16_t bufSize = KESPR::Display::Width() * KESPR::Display::Height() * sizeof(lv_color_t);
  lv_color_t *buf1 = reinterpret_cast<lv_color_t *>(heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  assert(buf1);
  lv_color_t *buf2 = reinterpret_cast<lv_color_t *>(heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  assert(buf2);

  lvDisp_ = lv_display_create(KESPR::Display::Width(), KESPR::Display::Height());
  assert(lvDisp_);
  lv_display_set_buffers(lvDisp_, buf1, buf2, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(lvDisp_, [](lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) -> void {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    KESPR::Display::PushPixels(area->x1, area->y1, w, h, reinterpret_cast<uint16_t *>(px_map));
    lv_disp_flush_ready(disp);
  });

  /* Create and start a periodic timer interrupt to call lv_tick_inc */
  const esp_timer_create_args_t periodic_timer_args = {
    .callback = [](void *arg) -> void {
      (void) arg;
      lv_tick_inc(CONFIG_KESPR_GUI_LVGL_TICK_MS);
    },
    .name = "lvgl_tick",
  };
  esp_timer_handle_t periodic_timer;
  err = esp_timer_create(&periodic_timer_args, &periodic_timer);
  ESP_RETURN_ON_ERROR(err, TAG, "create lvgl_tick timer");

  err = esp_timer_start_periodic(periodic_timer, CONFIG_KESPR_GUI_LVGL_TICK_MS * 1000);
  ESP_RETURN_ON_ERROR(err, TAG, "start lvgl_tick timer");

  xTaskCreatePinnedToCore(
    lvGuiTask,
    "lvgl_gui_task",
    CONFIG_KESPR_GUI_LVGL_TASK_STACK,
    nullptr,
    tskIDLE_PRIORITY+CONFIG_KESPR_GUI_LVGL_PRIO,
    nullptr,
    CONFIG_KESPR_GUI_LVGL_PIN_CORE
  );
  return ESP_OK;
}
