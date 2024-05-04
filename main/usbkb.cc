#include "usbkb.h"

#include <kespr_gui.h>

esp_err_t UsbKbApp::Start()
{
  if (this->Started()) {
    return ESP_OK;
  }

  KESPR::GUI::ChangeApp(KESPR::GUI::App::USBKb);
  KESPR::GUI::ChangeAppState(KESPR::GUI::AppState::Active);
  return AppsMan::App::Start();
}

esp_err_t UsbKbApp::Stop()
{
  if (!this->Started()) {
    return ESP_OK;
  }

  return AppsMan::App::Stop();
}
