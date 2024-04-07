#pragma once
#include <string>
#include <esp_err.h>
#include <esp_http_server.h>


namespace Service
{
  class BaseService
  {
  public:
    explicit BaseService(std::string name)
      : name_(std::move(name))
    {};

    virtual std::string Name();
    virtual bool Started();
    virtual esp_err_t Initialize(httpd_handle_t server);
    virtual esp_err_t Start();
    virtual esp_err_t Stop();

  protected:
    std::string name_ = "";
    bool started_ = false;
    httpd_handle_t httpd_ = nullptr;
  };
}
