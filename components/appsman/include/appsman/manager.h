#pragma once
#include <string>
#include <map>
#include <functional>

#include <esp_err.h>

#include "app.h"


namespace AppsMan::Manager
{
  esp_err_t Initialize();

  esp_err_t RegisterApp(App *app);
  App* CurrentApp();
  void WalkApps(std::function<void(const std::string& name, const App *app)> fn);

  std::map<std::string, MsgHandler> Handlers();

  esp_err_t SwitchApp(const std::string& appName);
  esp_err_t HandleRequest(int sockfd, const std::basic_string_view<uint8_t> payload);
}
