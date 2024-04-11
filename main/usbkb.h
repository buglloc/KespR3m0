#pragma once
#include <esp_err.h>

#include <ArduinoJson.h>
#include <httpd/app.h>


class UsbKbApp final : public HttpD::App
{
public:
  explicit UsbKbApp() : HttpD::App("usbkb") {};
};
