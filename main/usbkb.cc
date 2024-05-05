#include "usbkb.h"


esp_err_t UsbKbApp::Start()
{
  if (this->Started()) {
    return ESP_OK;
  }

  return AppsMan::App::Start();
}

esp_err_t UsbKbApp::Stop()
{
  if (!this->Started()) {
    return ESP_OK;
  }

  return AppsMan::App::Stop();
}
