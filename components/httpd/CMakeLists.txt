set(HTTPD_SRCS
  "src/apps_manager.cc"
  "src/events.cc"
  "src/server.cc"
  "src/static_handler.cc"
  "src/ws_commander.cc"
  "src/ws_handler.cc"
  "src/ws_keep_alive.cc"
  "src/ws_utils.cc"
)

idf_component_register(
SRCS
  ${HTTPD_SRCS}
INCLUDE_DIRS
  "include"
REQUIRES
  arduinojson esp_http_server
PRIV_REQUIRES
  defer esp_timer spiffs vfs log
)
