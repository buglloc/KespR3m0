#include "dispatcher.h"
#include <string>
#include <functional>
#include <format>

#include <esp_check.h>
#include <esp_log.h>

#include <httpd/server.h>
#include <defer.h>


using namespace AppsMan;
namespace
{
  static const char *TAG = "appsman::dispatcher";
  static const char *kMsgIdKey = "id";
  static const char *kMsgKindKey = "kind";
  static const char *kMsgErrorKey = "error";
  static const char *kMsgErrorCodeKey = "error_code";

  template <class... Types>
  constexpr inline int UNUSED(Types&&...) {
      return 0;
  }

  esp_err_t sendResponse(int sockfd, JsonObject& rspJson)
  {
    if (!rspJson.containsKey(kMsgKindKey)) {
      rspJson[kMsgKindKey] = "dummy";
      return HttpD::SendMsg(sockfd, rspJson);
    }

    return HttpD::SendMsg(sockfd, rspJson);
  }
}

Dispatcher::Dispatcher()
  : handlers_({
    {"ping", BIND_MSG_HANDLER(Dispatcher::HandlePing, this)},
    {"state", BIND_MSG_HANDLER(Dispatcher::HandleState, this)},
    {"app.state", BIND_MSG_HANDLER(Dispatcher::HandleAppState, this)},
    {"app.switch", BIND_MSG_HANDLER(Dispatcher::HandleAppSwitch, this)},
    {"app.setup", BIND_MSG_HANDLER(Dispatcher::HandleAppSetup, this)},
  })
{};

esp_err_t Dispatcher::HandlePing(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  UNUSED(sockfd, reqJson);
  rspJson[kMsgKindKey] = "pong";
  return ESP_OK;
}

esp_err_t Dispatcher::HandleState(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  UNUSED(sockfd, reqJson);
  return this->RespondState(rspJson);
}

esp_err_t Dispatcher::HandleAppState(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  return ESP_OK;
}

esp_err_t Dispatcher::HandleAppSwitch(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  const std::string& appName = reqJson["app"].as<std::string>();
  if (appName.empty()) {
    rspJson[kMsgErrorKey] = "no req[app]";
    rspJson[kMsgErrorCodeKey] = (int)ESP_ERR_INVALID_ARG;
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t err = Manager::SwitchApp(appName);
  if (err != ESP_OK) {
    rspJson[kMsgErrorKey] = std::format("start failed: {}", esp_err_to_name(err));
    rspJson[kMsgErrorCodeKey] = (int)ESP_FAIL;
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "app started: %s", appName.c_str());
  appHandlers_ = Manager::Handlers();
  return this->RespondState(rspJson);
}

esp_err_t Dispatcher::HandleAppSetup(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  return ESP_OK;
}

esp_err_t Dispatcher::RespondState(JsonObject& rspJson)
{
  rspJson[kMsgKindKey] = "state";
  JsonObject appsJson = rspJson["apps"].to<JsonObject>();
  Manager::WalkApps([&appsJson](const std::string name, const App *app) {
    JsonObject appJson = appsJson[name].to<JsonObject>();
    appJson["started"] = app->Started();
  });
  return ESP_OK;
}

AppsMan::MsgHandler Dispatcher::Handler(const std::string& name)
{
  {
    auto const& hi = this->handlers_.find(name);
    if (hi != this->handlers_.end()) {
      return hi->second;
    }
  }

  {
    auto const& hi = this->appHandlers_.find(name);
    if (hi != this->appHandlers_.end()) {
      return hi->second;
    }
  }

  return nullptr;
}

esp_err_t Dispatcher::Dispatch(int sockfd, std::basic_string_view<uint8_t> payload)
{
  JsonDocument rspDoc;
  JsonObject rspJson = rspDoc.to<JsonObject>();
  REF_DEFER(sendResponse(sockfd, rspJson));

  JsonDocument reqJson;
  DeserializationError jsonErr = deserializeJson(reqJson, payload);
  if (jsonErr && jsonErr != DeserializationError::Code::EmptyInput) {
    ESP_LOGE(TAG, "request parse failed (fd=%d): %s", sockfd, jsonErr.c_str());

    rspJson[kMsgErrorKey] = std::format("parse request: {}", jsonErr.c_str());
    rspJson[kMsgErrorCodeKey] = (int)ESP_ERR_INVALID_ARG;
    return ESP_ERR_INVALID_ARG;
  }

  rspJson[kMsgIdKey] = reqJson[kMsgIdKey];
  const std::string& msgKind = reqJson[kMsgKindKey].as<std::string>();
  if (msgKind.empty()) {
    ESP_LOGE(TAG, "no msg kind provided (fd=%d)", sockfd);

    rspJson[kMsgErrorKey] = "no req[kind]";
    rspJson[kMsgErrorCodeKey] = (int)ESP_ERR_INVALID_ARG;
    return ESP_ERR_INVALID_ARG;
  }

  auto const& handler = this->Handler(msgKind);
  if (handler == nullptr) {
    ESP_LOGE(TAG, "unknown msg (fd=%d): %s", sockfd, msgKind.c_str());

    rspJson[kMsgErrorKey] = std::format("unsupported msg kind: %d", msgKind);
    rspJson[kMsgErrorCodeKey] = (int)ESP_ERR_NOT_FOUND;
    return ESP_ERR_NOT_FOUND;
  }

  esp_err_t err = handler(sockfd, reqJson.as<JsonObjectConst>(), rspJson);
  if (err != ESP_OK) {
    ESP_LOGE(TAG,
      "msg handler failed (fd=%d, cmd=%s): %s",
      sockfd, msgKind.c_str(), esp_err_to_name(err)
    );

    rspJson[kMsgErrorKey] = esp_err_to_name(err);
    rspJson[kMsgErrorCodeKey] = (int)err;
    return ESP_FAIL;
  }

  return ESP_OK;
}
