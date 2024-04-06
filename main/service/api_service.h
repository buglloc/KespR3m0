#pragma once
#include <map>
#include <string>
#include <esp_err.h>
#include <esp_http_server.h>

#include "base_service.h"


namespace Service
{
  class ApiService final : public BaseService
  {
  public:
    struct Context
    {
      std::map<std::string, BaseService*> services;
      std::string curService;
      char scratch[CONFIG_KESPR_HTTP_BUF_SIZE];
    };

  public:
    explicit ApiService()
      : BaseService("api")
    {};

    esp_err_t Initialize(httpd_handle_t server) override;
    esp_err_t RegisterService(BaseService *svc);

  private:
    Context ctx = {};
  };
}
