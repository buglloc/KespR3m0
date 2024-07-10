idf_component_register(
SRCS
  "src/server.cc"
  "src/static_handler.cc"
  "src/ws_handler.cc"
  "src/ws_utils.cc"
INCLUDE_DIRS
  "include"
PRIV_INCLUDE_DIRS
  "priv_include"
REQUIRES
  arduinojson esp_http_server
PRIV_REQUIRES
  esp_timer heap spiffs vfs log
  scope_guard
)
