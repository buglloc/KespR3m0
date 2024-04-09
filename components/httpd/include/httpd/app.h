#pragma once
#include <string>
#include <functional>
#include <map>

#include <esp_err.h>
#include <esp_http_server.h>

#include <ArduinoJson.h>

#define BIND_COMMAND_HANDLER(h, this) std::bind(&h, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

namespace HttpD
{
  using CommandHandler = std::function<esp_err_t(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson)>;

  class App
  {
  public:
    explicit App(std::string name)
      : name_(std::move(name))
      , started_(false)
    {};

    virtual std::string Name() const {
      return name_;
    }

    virtual bool Started() const {
      return this->started_;
    }

    virtual esp_err_t Start(httpd_handle_t server) {
      if (this->started_) {
        return ESP_ERR_INVALID_STATE;
      }

      (void)(server);
      this->started_ = true;
      return ESP_OK;
    }

    virtual esp_err_t Stop(httpd_handle_t server) {
      if (!this->started_) {
        return ESP_ERR_INVALID_STATE;
      }

      (void)(server);
      this->started_ = false;
      return ESP_OK;
    }

    virtual std::map<std::string, CommandHandler> Commands() {
      return {};
    }

  protected:
    std::string EventName(const std::string& name) {
      return name_ + "." + name;
    }

  protected:
    std::string name_;
    bool started_;
  };
}
