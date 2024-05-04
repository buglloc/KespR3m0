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
  defer esp_timer spiffs vfs log
)
