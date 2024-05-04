#pragma once
#include <esp_err.h>

#include <ArduinoJson.h>
#include <appsman/app.h>


class UsbKbApp final : public AppsMan::App
{
public:
  explicit UsbKbApp() : AppsMan::App("usbkb") {};

  esp_err_t Start() override;
  esp_err_t Stop() override;
};
