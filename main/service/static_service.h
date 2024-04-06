#pragma once
#include <esp_err.h>
#include <esp_http_server.h>

#include "base_service.h"


namespace Service
{
  class StaticService final : public BaseService
  {
  public:
    struct Context
    {
      char scratch[CONFIG_KESPR_HTTP_BUF_SIZE];
    };

  public:
    explicit StaticService()
      : BaseService("static")
    {};

    esp_err_t Initialize(httpd_handle_t server) override;

  private:
    Context ctx = {};
  };
}
