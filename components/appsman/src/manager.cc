#include "appsman/manager.h"
#include <string>
#include <sstream>
#include <functional>
#include <format>

#include <esp_check.h>
#include <esp_log.h>

#include <httpd/server.h>
#include <defer.h>

#include "dispatcher.h"
#include "dummy_app.h"


using namespace AppsMan;
namespace
{
  static const char *TAG = "appsman::manager";
  static Dispatcher dispatcher_ = {};
  static DummyApp dummyApp_ = {};
  static std::map<std::string, App*> apps_ = {};
  static App* current_ = &dummyApp_;

  App* FindApp(const std::string& name)
  {
    auto const &appItem = apps_.find(name);
    if (appItem == apps_.end()) {
      return nullptr;
    }

    return appItem->second;
  }
}

esp_err_t Manager::Register(App *app)
{
  if (FindApp(app->Name()) != nullptr) {
    ESP_LOGE(TAG, "app '%s' already registered", app->Name().c_str());
    return ESP_ERR_INVALID_ARG;
  }

  apps_[app->Name()] = app;
  return ESP_OK;
}

const App* Manager::Current()
{
  return current_;
}

void Manager::Walk(std::function<void(const std::string& name, const App *app)> fn)
{
  for (auto&& [name, app] : apps_) {
    fn(name, app);
  }
}

std::map<std::string, MsgHandler> Manager::Handlers()
{
  std::map<std::string, MsgHandler> out;
  std::string prefix = current_->Name() + ".";
  for (auto&& [key, handler]: current_->Handlers()) {
    out[prefix + key] = handler;
  }

  return out;
}

esp_err_t Manager::Start(const std::string& appName)
{
  App* app = FindApp(appName);
  if (app == nullptr) {
    return ESP_ERR_NOT_FOUND;
  }

  esp_err_t err = app->Start();
  ESP_RETURN_ON_ERROR(err, TAG, "start app: %s", esp_err_to_name(err));

  current_ = app;
  return ESP_OK;
}

esp_err_t Manager::Stop()
{
  esp_err_t err = current_->Stop();
  current_ = &dummyApp_;
  return err;
}

esp_err_t Manager::HandleRequest(int sockfd, const std::basic_string_view<uint8_t> payload)
{
  return dispatcher_.Dispatch(sockfd, payload);
}
