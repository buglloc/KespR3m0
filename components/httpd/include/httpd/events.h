#pragma once
#include <string>
#include <esp_err.h>
#include <esp_http_server.h>

#include <ArduinoJson.h>


namespace HttpD
{
  void BuildEvent(const std::string& name, JsonObject& event);
  esp_err_t BroadcastEvent(httpd_handle_t server, const JsonObjectConst& event);
  esp_err_t SendEvent(httpd_handle_t server, int fd, const JsonObjectConst& event);
}
