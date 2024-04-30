idf_component_register(
SRCS
  "src/kespr_gui.cc"
  "src/kespr_lvgl.cc"
INCLUDE_DIRS
  "include"
PRIV_INCLUDE_DIRS
  "priv_include"
PRIV_REQUIRES
  esp_timer heap esp_hw_support
  driver lvgl
  board
)
