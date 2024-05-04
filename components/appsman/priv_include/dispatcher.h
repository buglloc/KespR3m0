#pragma once
#include <string>
#include <map>

#include <esp_err.h>
#include <esp_http_server.h>

#include <ArduinoJson.h>

#include "appsman/app.h"
#include "appsman/manager.h"


namespace AppsMan
{
  class Dispatcher
  {
  public:
    Dispatcher();
    esp_err_t Dispatch(int sockfd, std::basic_string_view<uint8_t> payload);

  protected:
    esp_err_t HandlePing(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson);
    esp_err_t HandleState(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson);
    esp_err_t HandleAppState(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson);
    esp_err_t HandleAppStart(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson);
    esp_err_t HandleAppSetup(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson);

  protected:
    esp_err_t RespondState(JsonObject& rspJson);
    MsgHandler Handler(const std::string& msgKind);

  protected:
    std::map<std::string, MsgHandler> handlers_;
    std::map<std::string, MsgHandler> appHandlers_;
  };
}
