#include <sdkconfig.h>
#include "kespr_gui.h"

#include <esp_check.h>

#include <lvgl.h>

#include "kespr_lvgl.h"
#include "kespr_display.h"


using namespace KESPR::GUI;

namespace
{
  static const char* TAG = "kespr::gui";

  void lvTick()
  {
    static bool ok = false;
    if (ok) {
      return;
    }

    ok = true;
  lv_obj_t *label = lv_label_create( lv_scr_act() );
  lv_label_set_text( label, "Hello world" );
  lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
  }
}

esp_err_t KESPR::GUI::Initialize()
{
  esp_err_t err = LVGL::Initialize();
  ESP_RETURN_ON_ERROR(err, TAG, "failed to initialize lvgl");

  // use lvgl timer instead of esp timer due to:
  // If you want to use a task to create the graphic, you NEED to create a Pinned task
  // Otherwise there can be problem such as memory corruption and so on.
  // source: https://github.com/lvgl/lv_port_esp32/blob/cffa173c6e410965da12875103b934ec9d28f4e5/main/main.c#L64-L66
  ESP_LOGI(TAG, "initialize ui task timer");
  lv_timer_create([](lv_timer_t *t) {
    lvTick();
  }, CONFIG_KESPR_GUI_PERIOD_TIME_MS, nullptr);

  KESPR::Display::SetBrightness(CONFIG_KESPR_DEFAULT_BRIGHTNESS);
  return ESP_OK;
}
