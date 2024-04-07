#pragma once
#include <string>
#include <esp_err.h>
#include <esp_http_server.h>

#include "base_service.h"
#include "ws_utils.h"


namespace Service::Ws
{
  class BaseService final : public ::Service::BaseService
  {
  public:
    explicit BaseService(std::string name)
      : ::Service::BaseService(std::move(name))
    {};

  protected:
    esp_err_t Broadcast(FrameBuilder builder, void *arg);
  };
}
