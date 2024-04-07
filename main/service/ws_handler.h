#pragma once
#include <string>

#include <esp_err.h>
#include <esp_http_server.h>

#include "bytes.h"


namespace Service::Ws
{
  class Handler
  {
  public:
    explicit Handler(std::string name)
      : name_(std::move(name))
    {};

    virtual std::string Name() {
      return name_;
    }

    virtual bool Startred() {
      return this->started_;
    }

    virtual void Start() {
      this->started_ = true;
    }

    virtual void Stop() {
      this->started_ = false;
    }

  protected:
    std::string name_;
    bool started_;
  };

  struct HandlerConfig
  {
    std::string uri;
    Handler *handler;
  };

  esp_err_t RegisterWsHandler(httpd_handle_t server, const HandlerConfig& cfg);
  esp_err_t UnRegisterWsHandler(httpd_handle_t server, const HandlerConfig& cfg);
}
