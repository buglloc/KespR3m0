idf_component_register(
SRCS
  "main.cc"
  "uart.cc"
  "usbkb.cc"
INCLUDE_DIRS
  "."
REQUIRES
  driver heap log arduinojson
  board httpd appsman gui
  scope_guard base64
)
