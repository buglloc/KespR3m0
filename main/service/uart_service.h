#pragma once
#include <esp_err.h>
#include <esp_http_server.h>

#include "base_service.h"
#include "ws_handler.h"


namespace Service
{
  class UartHandler final : public Ws::Handler
  {
  public:
    UartHandler(): Ws::Handler("uart") {};
  };

  class UartService final : public BaseService
  {
  public:
    UartService(): BaseService("uart") {};

    esp_err_t Initialize(httpd_handle_t server) override;
    esp_err_t Start() override;
    esp_err_t Stop() override;

  private:
    UartHandler handler_ = {};
  };
}
