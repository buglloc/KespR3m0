#pragma once
#include <esp_err.h>
#include <esp_http_server.h>

#include "base_service.h"
#include "ws_handler.h"


namespace Service
{
  class UsbkbHandler final : public Ws::Handler
  {
  public:
    UsbkbHandler() : Ws::Handler("usbkb") {};
  };

  class UsbkbService final : public BaseService
  {
  public:
    UsbkbService() : BaseService("usbkb") {};

    esp_err_t Initialize(httpd_handle_t server) override;
    esp_err_t Start() override;
    esp_err_t Stop() override;

  private:
    UsbkbHandler handler_ = {};
  };
}
