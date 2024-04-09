#pragma once
#include <esp_err.h>

#include <ArduinoJson.h>
#include <httpd/app.h>


class UartApp final : public HttpD::App
{
public:
  struct Context {
    httpd_handle_t server;
    bool started;
  };

public:
  explicit UartApp() : HttpD::App("uart") {};

  std::map<std::string, HttpD::CommandHandler> Commands() override;
  esp_err_t Start(httpd_handle_t server) override;
  esp_err_t Stop(httpd_handle_t server) override;

protected:
  esp_err_t HandleTx(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson);

protected:
  Context ctx_ = {};
};
