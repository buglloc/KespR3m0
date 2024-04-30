idf_component_register(
SRCS
  "src/manager.cc"
INCLUDE_DIRS
  "include"
REQUIRES
  driver
  lvgl
PRIV_REQUIRES
  esp_timer heap esp_hw_support
)

