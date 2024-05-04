#pragma once
#include <esp_err.h>

#include <ArduinoJson.h>
#include <appsman/app.h>


class UartApp final : public AppsMan::App
{
public:
  struct Context {
    bool started;
  };

public:
  explicit UartApp() : AppsMan::App("uart") {};

  std::map<std::string, AppsMan::MsgHandler> Handlers() override;
  esp_err_t Start() override;
  esp_err_t Stop() override;

protected:
  esp_err_t HandleTx(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson);

protected:
  Context ctx_ = {};
  uint8_t *txBuf = nullptr;
};
