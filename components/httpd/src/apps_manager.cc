#include "apps_manager.h"

#include <esp_log.h>


using namespace HttpD;
namespace {
  static const char *TAG = "httpd::apps";
}

App* AppsManager::Get(const std::string& name) const
{
  auto const &appItem = this->apps_.find(name);
  if (appItem == this->apps_.end()) {
    return nullptr;
  }

  return appItem->second;
}

std::map<std::string, CommandHandler> AppsManager::Commands(App* app) const
{
  if (app == nullptr) {
    return {};
  }

  std::map<std::string, CommandHandler> out;
  std::string prefix = app->Name() + ".";
  for (auto const& [key, handler]: app->Commands()) {
    out[prefix + key] = handler;
  }

  return out;
}

std::map<std::string, CommandHandler> AppsManager::Commands(const std::string& name) const
{
  App* app = this->Get(name);
  if (app == nullptr) {
    return {};
  }

  return this->Commands(app);
}

void AppsManager::Walk(std::function<void(const std::string& name, App *app)> fn) const
{
  for (auto const& [name, app] : this->apps_) {
    fn(name, app);
  }
}

esp_err_t AppsManager::Register(App *app)
{
  if (this->Get(app->Name()) != nullptr) {
    ESP_LOGE(TAG, "app '%s' already registered", app->Name().c_str());
    return ESP_ERR_INVALID_ARG;
  }

  this->apps_[app->Name()] = app;
  return ESP_OK;
}
