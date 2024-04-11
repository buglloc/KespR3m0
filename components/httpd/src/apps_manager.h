#pragma once
#include <string>
#include <map>
#include <functional>

#include <esp_err.h>
#include <esp_http_server.h>

#include "httpd/app.h"


namespace HttpD
{
  class AppsManager
  {
  public:
    esp_err_t Register(App*);
    App* Get(const std::string& name) const;
    std::map<std::string, CommandHandler> Commands(App* app) const;
    std::map<std::string, CommandHandler> Commands(const std::string& name) const;
    void Walk(std::function<void(const std::string& name, App *app)> fn) const;

  protected:
    std::map<std::string, App*> apps_ = {};
    App* current_ = nullptr;
  };
}
