file(GLOB_RECURSE IMAGE_SOURCES assets/images/*.c)
file(GLOB_RECURSE FONT_SOURCES assets/fonts/*.c)

idf_component_register(
SRCS
  "src/kespr_gui.cc"
  "src/kespr_lvgl.cc"
  "src/kespr_scene.cc"
  ${IMAGE_SOURCES}
  ${FONT_SOURCES}
INCLUDE_DIRS
  "include"
PRIV_INCLUDE_DIRS
  "priv_include"
PRIV_REQUIRES
  esp_timer heap esp_hw_support
  driver lvgl
  board appsman scope_guard
)
