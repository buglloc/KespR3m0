#pragma once
#include <string>
#include <functional>
#include <map>

#include <esp_err.h>

#include <ArduinoJson.h>


#define BIND_MSG_HANDLER(h, this) std::bind(&h, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

namespace AppsMan
{
  using MsgHandler = std::function<esp_err_t(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson)>;

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

    virtual esp_err_t Start() {
      this->started_ = true;
      return ESP_OK;
    }

    virtual esp_err_t Stop() {
      this->started_ = false;
      return ESP_OK;
    }

    virtual std::map<std::string, MsgHandler> Handlers() {
      return {};
    }

  protected:
    std::string MsgName(const std::string& name) {
      return name_ + "." + name;
    }

  protected:
    std::string name_;
    bool started_;
  };
}
