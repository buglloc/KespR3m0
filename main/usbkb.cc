#include "usbkb.h"

#include <httpd/events.h>
#include <kespr_gui.h>

esp_err_t UsbKbApp::Start(httpd_handle_t server)
{
  if (this->started_) {
    return ESP_ERR_INVALID_STATE;
  }

  KESPR::GUI::ChangeApp(KESPR::GUI::App::USBKb);
  return HttpD::App::Start(server);
}

esp_err_t UsbKbApp::Stop(httpd_handle_t server)
{
  if (!this->started_) {
    return ESP_ERR_INVALID_STATE;
  }

  return HttpD::App::Stop(server);
}
