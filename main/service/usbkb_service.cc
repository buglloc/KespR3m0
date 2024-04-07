#include "usbkb_service.h"
#include <string>

#include <esp_check.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <defer.h>

#include "ws_handler.h"


using namespace Service;
namespace {
  static const char *TAG = "usbkb_svc";
}

esp_err_t UsbkbService::Initialize(httpd_handle_t server)
{
  Ws::HandlerConfig cfg = {
    .uri = "/usbkb/ws",
    .handler = &this->handler_,
  };
  esp_err_t err = Ws::RegisterWsHandler(server, cfg);
  ESP_RETURN_ON_ERROR(err, TAG, "register ws handler: %s", esp_err_to_name(err));

  return BaseService::Initialize(server);
}

esp_err_t UsbkbService::Start()
{
  if (this->started_) {
    return ESP_OK;
  }

  this->handler_.Start();
  return BaseService::Start();
}

esp_err_t UsbkbService::Stop()
{
  if (!this->started_) {
    return ESP_OK;
  }

  this->handler_.Stop();
  return BaseService::Stop();
}
