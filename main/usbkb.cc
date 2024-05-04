#include "usbkb.h"

#include <kespr_gui.h>

esp_err_t UsbKbApp::Start()
{
  if (this->Started()) {
    return ESP_ERR_INVALID_STATE;
  }

  KESPR::GUI::ChangeApp(KESPR::GUI::App::USBKb);
  KESPR::GUI::ChangeAppState(KESPR::GUI::AppState::Active);
  return AppsMan::App::Start();
}

esp_err_t UsbKbApp::Stop()
{
  if (!this->Started()) {
    return ESP_ERR_INVALID_STATE;
  }

  return AppsMan::App::Stop();
}
