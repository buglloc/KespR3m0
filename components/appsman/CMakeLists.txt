idf_component_register(
SRCS
  "src/dispatcher.cc"
  "src/manager.cc"
INCLUDE_DIRS
  "include"
PRIV_INCLUDE_DIRS
  "priv_include"
REQUIRES
  arduinojson httpd
PRIV_REQUIRES
  scope_guard
)
