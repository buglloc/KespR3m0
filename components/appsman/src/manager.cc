#include "appsman/manager.h"

#include <string>
#include <sstream>
#include <functional>
#include <format>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <esp_check.h>
#include <esp_log.h>
#include <esp_event.h>

#include <httpd/server.h>
#include <defer.h>

#include "dispatcher.h"
#include "dummy_app.h"


using namespace AppsMan;
namespace
{
  static const char *TAG = "appsman::manager";
  static QueueHandle_t incomingQueue_ = nullptr;
  static Dispatcher dispatcher_ = {};
  static DummyApp dummyApp_ = {};
  static std::map<std::string, App*> apps_ = {};
  static App* current_ = &dummyApp_;

  typedef struct {
    int sockfd;
    uint8_t *data;
    size_t size;
  } IncomingMsg;

  App* FindApp(const std::string& name)
  {
    auto const &appItem = apps_.find(name);
    if (appItem == apps_.end()) {
      return nullptr;
    }

    return appItem->second;
  }

  void dispatchWorker(void *arg)
  {
    (void)(arg);
    ESP_LOGI(TAG, "dispatch worker started");
    for (;;) {
      IncomingMsg msg;
      if (!xQueueReceive(incomingQueue_, &msg, portMAX_DELAY)) {
        continue;
      }

      std::basic_string_view<uint8_t> payload{msg.data, msg.size};
      esp_err_t err = dispatcher_.Dispatch(msg.sockfd, payload);
      heap_caps_free(msg.data);

      if (err != ESP_OK) {
        ESP_LOGE(TAG, "dispatch failed: %s", esp_err_to_name(err));
        continue;
      }
    }

    ESP_LOGW(TAG, "dispatch worker stopped");
    vTaskDelete(nullptr);
  }
}

esp_err_t Manager::Initialize()
{
  incomingQueue_ = xQueueCreate(CONFIG_KESPR_APPS_DISPATCH_QUEUE, sizeof(IncomingMsg));
  if (incomingQueue_ == nullptr){
    ESP_LOGE(TAG, "failed to create dispatch queue");
    return ESP_ERR_NO_MEM;
  }

  for (int i = 0; i < CONFIG_KESPR_APPS_DISPATCH_WORKERS; ++i) {
    bool ok = xTaskCreate(
      dispatchWorker,
      "appsman_disp_task",
      CONFIG_KESPR_APPS_TASK_STACK_SIZE,
      nullptr,
      tskIDLE_PRIORITY+CONFIG_KESPR_APPS_TASK_PRIORITY,
      nullptr
    );

    if (!ok) {
      ESP_LOGE(TAG, "failed to start dispatch worker");
      return ESP_FAIL;
    }
  }

  return ESP_OK;
}

esp_err_t Manager::RegisterApp(App *app)
{
  if (FindApp(app->Name()) != nullptr) {
    ESP_LOGE(TAG, "app '%s' already registered", app->Name().c_str());
    return ESP_ERR_INVALID_ARG;
  }

  apps_[app->Name()] = app;
  return ESP_OK;
}

App* Manager::CurrentApp()
{
  return current_;
}

void Manager::WalkApps(std::function<void(const std::string& name, const App *app)> fn)
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

esp_err_t Manager::SwitchApp(const std::string& appName)
{
  if (Manager::CurrentApp()->Name() == appName) {
    return ESP_OK;
  }

  App* app = FindApp(appName);
  if (app == nullptr) {
    return ESP_ERR_NOT_FOUND;
  }

  esp_err_t err = ESP_OK;

  App* curApp = Manager::CurrentApp();
  ESP_LOGI(TAG, "stop app: %s", curApp->Name().c_str());
  err = curApp->Stop();
  ESP_RETURN_ON_ERROR(err, TAG, "stop current app: %s", esp_err_to_name(err));

  ESP_LOGI(TAG, "start app: %s", app->Name().c_str());
  err = app->Start();
  ESP_RETURN_ON_ERROR(err, TAG, "start app: %s", esp_err_to_name(err));

  ESP_LOGI(TAG, "app started: %s", app->Name().c_str());
  current_ = app;
  return ESP_OK;
}

esp_err_t Manager::HandleRequest(int sockfd, const std::basic_string_view<uint8_t> payload)
{
  IncomingMsg msg = {
    .sockfd = sockfd,
    .size = payload.size(),
  };
  msg.data = reinterpret_cast<uint8_t *>(heap_caps_malloc(payload.size(), MALLOC_CAP_SPIRAM));
  if (msg.data == nullptr) {
    ESP_LOGE(TAG, "no mem to allocate payload");
    return ESP_ERR_NO_MEM;
  }
  memcpy(msg.data, payload.data(), payload.size());

  if (xQueueSend(incomingQueue_, &msg, pdMS_TO_TICKS(CONFIG_KESPR_APPS_DISPATCH_TIMEOUT_MS)) == false) {
    ESP_LOGE(TAG, "worker queue is full");
    heap_caps_free(msg.data);
    return ESP_FAIL;
  }

  return ESP_OK;
}
