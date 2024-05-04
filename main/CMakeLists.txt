idf_component_register(
SRCS
  "main.cc"
  "uart.cc"
  "usbkb.cc"
INCLUDE_DIRS
  "."
REQUIRES
  driver log arduinojson
  board httpd appsman gui
  defer base64
)
