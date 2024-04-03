idf_component_register(
SRCS
  "main.cc"
  "service/service.cc"
  "service/api_service.cc"
  "service/static_service.cc"
  "service/keyboard_service.cc"
  "service/uart_service.cc"
INCLUDE_DIRS
  "."
REQUIRES
  spiffs esp_http_server json vfs log
  ui board defer LovyanGFX
)
