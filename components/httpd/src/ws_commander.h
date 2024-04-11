#pragma once
#include <string>
#include <map>

#include <esp_err.h>
#include <esp_http_server.h>

#include <ArduinoJson.h>

#include "apps_manager.h"


namespace HttpD::Ws
{
  class Commander
  {
  public:
    explicit Commander(AppsManager* appsManager);
    esp_err_t Dispatch(httpd_req_t *req, std::basic_string_view<uint8_t> payload);

  protected:
    esp_err_t HandlePing(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson);
    esp_err_t HandleState(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson);
    esp_err_t HandleAppState(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson);
    esp_err_t HandleAppStart(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson);
    esp_err_t HandleAppSetup(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson);

  protected:
    esp_err_t RespondState(JsonObject& rspJson);
    CommandHandler GetHandler(const std::string& name);

  protected:
    AppsManager* appsManager_;
    std::map<std::string, CommandHandler> handlers_;
    std::map<std::string, CommandHandler> appHandlers_;
  };
}
