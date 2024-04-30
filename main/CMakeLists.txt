idf_component_register(
SRCS
  "main.cc"
  "uart.cc"
  "usbkb.cc"
INCLUDE_DIRS
  "."
REQUIRES
  log arduinojson
  ui board httpd defer base64
)
