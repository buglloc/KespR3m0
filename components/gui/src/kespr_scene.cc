#include "kespr_scene.h"
#include <utility>

#include <lvgl.h>


LV_FONT_DECLARE(ui_font_icons);
LV_FONT_DECLARE(ui_font_regular);
LV_FONT_DECLARE(ui_font_title);

LV_IMG_DECLARE(ui_img_kespremo_bg_full);
LV_IMG_DECLARE(ui_img_kespremo_bg_empty);
LV_IMG_DECLARE(ui_img_kespremo_bg_white);
LV_IMG_DECLARE(ui_img_kespremo_bg_blue);

#define UI_ARROW_UP_SYMBOL   "\xF3\xB0\x9C\xB7"
#define UI_ARROW_DOWN_SYMBOL "\xF3\xB0\x9C\xB7"
#define UI_WIFI_SYMBOL       "\xF3\xB0\x96\xA9"
#define UI_BATT_SYMBOL       "\xF3\xB0\x84\x8C"


using namespace KESPR::GUI;
namespace
{
  const lv_color_t kColorText = lv_color_hex(0xffffff);
  const lv_color_t kColorActive = lv_color_hex(0x72C0FD);
  const lv_color_t kColorPanel = lv_color_hex(0x212121);
  constexpr lv_obj_flag_t kFlagsAllStatic =
    LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
    LV_OBJ_FLAG_SNAPPABLE | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC |
    LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN;
  constexpr lv_obj_flag_t kFlagsAll =
    LV_OBJ_FLAG_CLICKABLE | kFlagsAllStatic;

  lv_style_t styleAppState_;
  lv_style_t styleAppTitle_;
  lv_style_t styleIcons_;
  lv_style_t styleStatusArc_;
  lv_style_t styleStatusArcIndicator_;
  lv_style_t styleStatusArcKnob_;

  lv_subject_t subjectApp_;
  lv_subject_t subjectAppState_;
  lv_subject_t subjectWiFiLevel_;
  lv_subject_t subjectBattLevel_;


  void initStyles()
  {
    // app state
    {
      lv_style_init(&styleAppState_);
      lv_style_set_size(&styleAppState_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_style_set_align(&styleAppState_, LV_ALIGN_CENTER);
    }

    // app title
    {
      lv_style_init(&styleAppTitle_);
      lv_style_set_size(&styleAppTitle_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_style_set_align(&styleAppTitle_, LV_ALIGN_CENTER);
      lv_style_set_text_font(&styleAppTitle_, &ui_font_title);
      lv_style_set_text_color(&styleAppTitle_, kColorActive);
      lv_style_set_text_align(&styleAppTitle_, LV_TEXT_ALIGN_CENTER);
    }

    // icons
    {
      lv_style_init(&styleIcons_);
      lv_style_set_size(&styleIcons_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_style_set_align(&styleIcons_, LV_ALIGN_CENTER);
      lv_style_set_text_font(&styleIcons_, &ui_font_icons);
      lv_style_set_text_color(&styleIcons_, kColorActive);
      lv_style_set_text_align(&styleIcons_, LV_TEXT_ALIGN_CENTER);
    }

    // status arc
    {
      lv_style_init(&styleStatusArc_);
      lv_style_set_size(&styleStatusArc_, lv_disp_get_physical_hor_res(nullptr), lv_disp_get_ver_res(nullptr));
      lv_style_set_pad_all(&styleStatusArc_, 6);
      lv_style_set_arc_color(&styleStatusArc_, kColorPanel);
      lv_style_set_arc_width(&styleStatusArc_, 6);

      lv_style_init(&styleStatusArcIndicator_);
      lv_style_set_arc_color(&styleStatusArcIndicator_, kColorActive);
      lv_style_set_arc_width(&styleStatusArcIndicator_, 6);
      lv_style_set_arc_rounded(&styleStatusArcIndicator_, true);

      lv_style_init(&styleStatusArcKnob_);
      lv_style_set_bg_opa(&styleStatusArcKnob_, LV_OPA_0);
    }
  }

  void initSubjects()
  {
    lv_subject_init_int(&subjectApp_, std::to_underlying(KESPR::GUI::App::None));
    lv_subject_init_int(&subjectAppState_, std::to_underlying(KESPR::GUI::AppState::Inactive));
    lv_subject_init_int(&subjectWiFiLevel_, 0);
    lv_subject_init_int(&subjectBattLevel_, 0);
  }

  lv_obj_t* createStatusArc(lv_obj_t *parent)
  {
    lv_obj_t *statusArc = lv_arc_create(parent);
    lv_obj_remove_style_all(statusArc);
    lv_obj_add_style(statusArc, &styleStatusArc_, LV_PART_MAIN);
    lv_obj_add_style(statusArc, &styleStatusArcIndicator_, LV_PART_INDICATOR);
    lv_obj_add_style(statusArc, &styleStatusArcKnob_, LV_PART_KNOB);
    lv_obj_clear_flag(statusArc, kFlagsAll);
    return statusArc;
  }

  void appStateObserverCb(lv_observer_t *observer, lv_subject_t *subject)
  {
    lv_obj_t *img = reinterpret_cast<lv_obj_t *>(lv_observer_get_target(observer));
    switch(static_cast<KESPR::GUI::AppState>(lv_subject_get_int(subject))) {
      case KESPR::GUI::AppState::Inactive:
        lv_img_set_src(img, &ui_img_kespremo_bg_empty);
        break;

      case KESPR::GUI::AppState::PartialLeft:
        lv_img_set_src(img, &ui_img_kespremo_bg_white);
        break;

      case KESPR::GUI::AppState::PartialRight:
        lv_img_set_src(img, &ui_img_kespremo_bg_blue);
        break;

      case KESPR::GUI::AppState::Active:
        lv_img_set_src(img, &ui_img_kespremo_bg_full);
        break;

      default:
        lv_img_set_src(img, &ui_img_kespremo_bg_empty);
        break;
    }
  }

  void appTitleObserverCb(lv_observer_t *observer, lv_subject_t *subject)
  {
    lv_obj_t *label = reinterpret_cast<lv_obj_t *>(lv_observer_get_target(observer));
    int32_t value = lv_subject_get_int(subject);

    switch(static_cast<KESPR::GUI::App>(value)) {
      case KESPR::GUI::App::USBKb:
        lv_label_set_text_static(label, "USB");
        break;

      case KESPR::GUI::App::UART:
        lv_label_set_text_static(label, "UART");
        break;

      case KESPR::GUI::App::None:
        lv_label_set_text_static(label, "<oO>");
        break;

      default:
        lv_label_set_text_fmt(label, "<%lu>", value);
        break;
    }
  }

static lv_obj_t *appTitle;
  lv_obj_t* createAppStater(lv_obj_t *parent)
  {
    lv_obj_t *stateImg = lv_img_create(parent);
    lv_obj_remove_style_all(stateImg);
    lv_obj_add_style(stateImg, &styleAppTitle_, LV_PART_MAIN);
    lv_obj_clear_flag(stateImg, kFlagsAllStatic);
    lv_subject_add_observer_obj(&subjectAppState_, appStateObserverCb, stateImg, nullptr);

    appTitle = lv_label_create(parent);
    lv_obj_remove_style_all(appTitle);
    lv_obj_add_style(appTitle, &styleAppTitle_, LV_PART_MAIN);
    lv_obj_clear_flag(appTitle, kFlagsAllStatic);
    lv_label_set_long_mode(appTitle, LV_LABEL_LONG_DOT);
    lv_subject_add_observer_obj(&subjectApp_, appTitleObserverCb, appTitle, nullptr);

    return appTitle;
  }
}

esp_err_t Scene::Show()
{
  initStyles();
  initSubjects();

  lv_obj_t *scene = lv_obj_create(lv_screen_active());
  lv_obj_set_size(scene, lv_disp_get_physical_hor_res(nullptr), lv_disp_get_ver_res(nullptr));
  lv_obj_remove_style(scene, 0, LV_PART_SCROLLBAR);
  lv_obj_clear_flag(scene, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(scene, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(scene, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(scene, lv_color_hex(0x000000), LV_PART_MAIN);

  lv_obj_t *stater = createAppStater(scene);
  LV_UNUSED(stater);

  lv_obj_t *battLevel = createStatusArc(scene);
  lv_arc_set_range(battLevel, 0, KESPR::GUI::Scene::kMaxLevel);
  lv_arc_set_bg_angles(battLevel, 0, 70);
  lv_arc_set_rotation(battLevel, 280);
  lv_arc_bind_value(battLevel, &subjectBattLevel_);

  lv_obj_t *battLabel = lv_label_create(scene);
  lv_obj_remove_style_all(battLabel);
  lv_obj_add_style(battLabel, &styleIcons_, LV_PART_MAIN);
  lv_obj_set_align(battLabel, LV_ALIGN_RIGHT_MID);
  lv_obj_clear_flag(battLabel, kFlagsAllStatic);
  lv_label_set_text_static(battLabel, UI_BATT_SYMBOL);
  lv_obj_set_style_pad_right(battLabel, 4, LV_PART_MAIN);

  lv_obj_t *wifiLevel = createStatusArc(scene);
  lv_arc_set_range(wifiLevel, 0, KESPR::GUI::Scene::kMaxLevel);
  lv_arc_set_bg_angles(wifiLevel, 0, 70);
  lv_arc_set_rotation(wifiLevel, 100);
  lv_arc_bind_value(wifiLevel, &subjectWiFiLevel_);

  lv_obj_t *wifiLabel = lv_label_create(scene);
  lv_obj_remove_style_all(wifiLabel);
  lv_obj_add_style(wifiLabel, &styleIcons_, LV_PART_MAIN);
  lv_obj_set_align(wifiLabel, LV_ALIGN_LEFT_MID);
  lv_obj_clear_flag(wifiLabel, kFlagsAllStatic);
  lv_label_set_text_static(wifiLabel, UI_WIFI_SYMBOL);
  lv_obj_set_style_pad_left(wifiLabel, 4, LV_PART_MAIN);

  return ESP_OK;
}

void Scene::SetApp(const KESPR::GUI::App app)
{
  lv_subject_set_int(&subjectApp_, static_cast<int32_t>(app));
}

void Scene::SetAppState(const KESPR::GUI::AppState state)
{
  lv_subject_set_int(&subjectAppState_, static_cast<int32_t>(state));
}

void Scene::SetBattLevel(const int32_t level)
{
  lv_subject_set_int(&subjectBattLevel_, level);
}

void Scene::SetWiFiLevel(const int32_t level)
{
  lv_subject_set_int(&subjectWiFiLevel_, level);
}
