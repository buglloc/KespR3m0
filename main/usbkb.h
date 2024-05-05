#pragma once
#include <utility>

#include <esp_err.h>

#include <ArduinoJson.h>
#include <appsman/app.h>
#include <kespr_gui.h>


class UsbKbApp final : public AppsMan::App
{
public:
  explicit UsbKbApp() : AppsMan::App(std::to_underlying(KESPR::GUI::App::USBKb), "usbkb") {};

  esp_err_t Start() override;
  esp_err_t Stop() override;
};
