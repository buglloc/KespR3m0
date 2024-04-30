idf_component_register(
SRCS
  "src/kespr_board.cc"
  "src/kespr_display.cc"
  "src/kespr_net.cc"

  "src/display_wshare_lcd.cc"
INCLUDE_DIRS
  "include"
PRIV_INCLUDE_DIRS
  "priv_include"
REQUIRES
  esp_netif
PRIV_REQUIRES
  esp_wifi esp_event nvs_flash
  dns_server LovyanGFX
)

# TODO(buglloc): fix me
idf_build_set_property(COMPILE_OPTIONS "-Wno-missing-field-initializers" APPEND)
