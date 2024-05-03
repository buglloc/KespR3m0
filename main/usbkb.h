#pragma once
#include <esp_err.h>

#include <ArduinoJson.h>
#include <httpd/app.h>


class UsbKbApp final : public HttpD::App
{
public:
  explicit UsbKbApp() : HttpD::App("usbkb") {};

  esp_err_t Start(httpd_handle_t server) override;
  esp_err_t Stop(httpd_handle_t server) override;
};
