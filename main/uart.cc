#include "uart.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include <base64.h>
#include <httpd/events.h>

#define USED_UART_NUM UART_NUM_2

namespace {
  static const char* TAG = "app::uart";
  static const char* RX_TAG = "app::uart::rx";
  const size_t kRxBufSize = CONFIG_KESPR_UARTD_BUF_SIZE;
  const size_t kRxBase64BufSize = BASE64_ENCODE_OUT_SIZE(CONFIG_KESPR_UARTD_BUF_SIZE);

  esp_err_t initUart()
  {
    const uart_config_t uart_config = {
      .baud_rate = CONFIG_KESPR_UARTD_DEFAULT_BRAUD,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
    };

    // We won't use a buffer for sending data.
    esp_err_t err = uart_driver_install(USED_UART_NUM, kRxBufSize * 2, 0, 0, nullptr, 0);
    ESP_RETURN_ON_ERROR(err, TAG, "instal driver");

    err = uart_param_config(USED_UART_NUM, &uart_config);
    ESP_RETURN_ON_ERROR(err, TAG, "configure driver");

    uart_set_pin(UART_NUM_2, CONFIG_KESPR_UARTD_TX_PIN, CONFIG_KESPR_UARTD_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_RETURN_ON_ERROR(err, TAG, "configure driver");

    return ESP_OK;
  }

  static void rxTask(void *arg)
  {
    ESP_LOGI(RX_TAG, "started");

    UartApp::Context *ctx = static_cast<UartApp::Context *>(arg);
    JsonDocument event;
    event["cmd"] = "uart.rx";

    uint8_t *rawData = reinterpret_cast<uint8_t *>(malloc(kRxBufSize * 2));
    assert(rawData);

    char *base64Data = reinterpret_cast<char *>(malloc(kRxBase64BufSize));
    assert(base64Data);

    while (ctx->started && ctx->server != nullptr) {
      const int rxBytes = uart_read_bytes(USED_UART_NUM, rawData, kRxBufSize - 1, CONFIG_KESPR_UARTD_READ_PERIOD / portTICK_PERIOD_MS);
      if (rxBytes <= 0) {
        continue;
      }

      base64_encode(rawData, rxBytes, base64Data);
      event["data"] = base64Data;
      HttpD::BroadcastEvent(ctx->server, event.as<JsonVariantConst>());
    }

    ESP_LOGI(RX_TAG, "stopped");
    free(rawData);
    free(base64Data);
  }
}

std::map<std::string, HttpD::CommandHandler> UartApp::Commands()
{
  return {
    {"tx", BIND_COMMAND_HANDLER(UartApp::HandleTx, this)}
  };
}

esp_err_t UartApp::Start(httpd_handle_t server)
{
  if (this->started_) {
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t err = initUart();
  if (err != ESP_OK) {
    uart_driver_delete(USED_UART_NUM);
    ESP_LOGE(TAG, "init uart: %s", esp_err_to_name(err));
    return err;
  }

  this->ctx_.server = server;
  this->ctx_.started = true;
  xTaskCreate(rxTask, "uart_rx_task", CONFIG_KESPR_UARTD_RX_STACK_SIZE, &this->ctx_, configMAX_PRIORITIES - 1, nullptr);

  return HttpD::App::Start(server);
}

esp_err_t UartApp::Stop(httpd_handle_t server)
{
  if (!this->started_) {
    return ESP_ERR_INVALID_STATE;
  }

  this->ctx_.started = false;
  esp_err_t err = uart_driver_delete(UART_NUM_2);
  ESP_RETURN_ON_ERROR(err, TAG, "delete uart");

  return HttpD::App::Stop(server);
}

esp_err_t UartApp::HandleTx(httpd_req_t *req, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  const std::string data = reqJson["data"];
  uart_write_bytes(USED_UART_NUM, data.c_str(), data.size());
  return ESP_OK;
}
