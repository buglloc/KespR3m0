idf_component_register(
SRCS
  "main.cc"
  "service/service.cc"
  "service/base_service.cc"
  "service/ws_handler.cc"
  "service/ws_utils.cc"
  "service/ws_base_service.cc"
  "service/ws_keep_alive.cc"
  "service/api_service.cc"
  "service/static_service.cc"
  "service/usbkb_service.cc"
  "service/uart_service.cc"
INCLUDE_DIRS
  "."
REQUIRES
  spiffs esp_http_server json vfs log
  ui board defer LovyanGFX
)
