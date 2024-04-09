#include "ws_commander.h"
#include <string>
#include <sstream>
#include <functional>

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <defer.h>


using namespace HttpD::Ws;
namespace
{
  static const char *TAG = "httpd::ws::commander";

  template <class... Types>
  constexpr inline int UNUSED(Types&&...) {
      return 0;
  }

  esp_err_t sendResponse(httpd_req_t *req, const JsonObject& rspJson)
  {
    if (!rspJson.containsKey("cmd")) {
      return ESP_OK;
    }

    std::string out;
    if (serializeJson(rspJson, out) == 0) {
      ESP_LOGE(TAG, "unable to serialize response");
      return ESP_FAIL;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)out.data();
    ws_pkt.len = out.size();
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    return httpd_ws_send_frame(req, &ws_pkt);
  }
}

Commander::Commander(AppsManager* appsManager)
  : appsManager_(appsManager)
  , handlers_({
    {"ping", BIND_COMMAND_HANDLER(Commander::HandlePing, this)},
    {"state", BIND_COMMAND_HANDLER(Commander::HandleState, this)},
    {"app.state", BIND_COMMAND_HANDLER(Commander::HandleAppState, this)},
    {"app.start", BIND_COMMAND_HANDLER(Commander::HandleAppStart, this)},
    {"app.setup", BIND_COMMAND_HANDLER(Commander::HandleAppSetup, this)},
  })
{};

esp_err_t Commander::HandlePing(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  UNUSED(req, reqJson);
  rspJson["cmd"] = "pong";
  return ESP_OK;
}

esp_err_t Commander::HandleState(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  UNUSED(req, reqJson);
  return this->RespondState(rspJson);
}

esp_err_t Commander::HandleAppState(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  return ESP_OK;
}

esp_err_t Commander::HandleAppStart(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  const std::string& appName = reqJson["app"].as<std::string>();
  if (appName.empty()) {
    rspJson["error"] = "no req[app]";
    rspJson["error_code"] = (int)ESP_ERR_INVALID_ARG;
    return ESP_ERR_INVALID_ARG;
  }

  ESP_LOGI(TAG, "staring app: %s", appName.c_str());
  App *app = this->appsManager_->Get(appName);
  if (app == nullptr) {
    rspJson["error"] = "unknown app requested";
    rspJson["error_code"] = (int)ESP_ERR_NOT_FOUND;
    return ESP_ERR_NOT_FOUND;
  }

  esp_err_t err = app->Start(req->handle);
  if (err != ESP_OK) {
    rspJson["error"] = "start failed";
    rspJson["error_code"] = (int)ESP_FAIL;
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "app started: %s", appName.c_str());
  appHandlers_ = this->appsManager_->Commands(app);
  return this->RespondState(rspJson);
}

esp_err_t Commander::HandleAppSetup(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  return ESP_OK;
}

esp_err_t Commander::RespondState(JsonObject& rspJson)
{
  rspJson["cmd"] = "state";
  JsonObject appsJson = rspJson["apps"].to<JsonObject>();
  this->appsManager_->Walk([&appsJson](const std::string name, const App *app) {
    JsonObject appJson = appsJson[app->Name()].to<JsonObject>();
    appJson["started"] = app->Started();
  });
  return ESP_OK;
}

HttpD::CommandHandler Commander::GetHandler(const std::string& name)
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

esp_err_t Commander::Dispatch(httpd_req_t *req, std::basic_string_view<uint8_t> payload)
{
  JsonDocument rspDoc;
  JsonObject rspJson = rspDoc.to<JsonObject>();
  REF_DEFER(sendResponse(req, rspJson));

  JsonDocument reqJson;
  DeserializationError jsonErr = deserializeJson(reqJson, payload);
  if (jsonErr && jsonErr != DeserializationError::Code::EmptyInput) {
    ESP_LOGE(TAG, "request parse failed (fd=%d): %s", httpd_req_to_sockfd(req), jsonErr.c_str());

    rspJson["error"] = "parse failed";
    rspJson["error_code"] = (int)ESP_ERR_INVALID_ARG;
    return ESP_ERR_INVALID_ARG;
  }

  rspJson["id"] = reqJson["id"];
  const std::string& cmd = reqJson["cmd"].as<std::string>();
  if (cmd.empty()) {
    ESP_LOGE(TAG, "no command provided (fd=%d)", httpd_req_to_sockfd(req));

    rspJson["error"] = "no req[cmd]";
    rspJson["error_code"] = (int)ESP_ERR_INVALID_ARG;
    return ESP_ERR_INVALID_ARG;
  }

  auto const& handler = this->GetHandler(cmd);
  if (handler == nullptr) {
    ESP_LOGE(TAG, "unknown command (fd=%d): %s", httpd_req_to_sockfd(req), cmd.c_str());

    rspJson["error"] = "command not found";
    rspJson["error_code"] = (int)ESP_ERR_NOT_FOUND;
    return ESP_ERR_NOT_FOUND;
  }

  esp_err_t err = handler(req, reqJson.as<JsonObjectConst>(), rspJson);
  if (err != ESP_OK) {
    ESP_LOGE(TAG,
      "command failed (fd=%d, cmd=%s): %s",
      httpd_req_to_sockfd(req), cmd.c_str(), esp_err_to_name(err)
    );

    rspJson["error"] = "command failed";
    rspJson["error_code"] = (int)err;
    return ESP_FAIL;
  }

  return ESP_OK;
}
