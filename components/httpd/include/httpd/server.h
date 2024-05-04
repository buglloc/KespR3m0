#pragma once
#include <string>
#include <functional>

#include <esp_err.h>
#include <esp_http_server.h>

#include <ArduinoJson.h>


namespace HttpD
{
  using WsMsgHandler = std::function<esp_err_t(int sockfd, const std::basic_string_view<uint8_t> payload)>;

  esp_err_t Start(const WsMsgHandler& msgHandler);

  esp_err_t BroadcastMsg(const JsonObjectConst& msg);
  esp_err_t SendMsg(int sockfd, const JsonObjectConst& msg);
}
