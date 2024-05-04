#include "uart.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_check.h>
#include <esp_heap_caps.h>

#include <driver/uart.h>
#include <driver/gpio.h>

#include <httpd/server.h>
#include <defer.h>
#include <base64.h>
#include <kespr_gui.h>


#define USED_UART_NUM UART_NUM_2

namespace {
  static const char* TAG = "app::uart";
  static const char* RX_TAG = "app::uart::rx";
  const size_t kRxBufSize = CONFIG_KESPR_UARTD_BUF_SIZE;
  const size_t kRxBase64BufSize = BASE64_ENCODE_OUT_SIZE(CONFIG_KESPR_UARTD_BUF_SIZE);
  const size_t kTxBufSize = BASE64_DECODE_OUT_SIZE(CONFIG_KESPR_UARTD_BUF_SIZE);

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
    JsonDocument msg;
    msg["kind"] = "uart.rx";

    uint8_t *rawData = reinterpret_cast<uint8_t *>(heap_caps_malloc(kRxBufSize * 2, MALLOC_CAP_SPIRAM));
    assert(rawData);
    REF_DEFER(heap_caps_free(rawData));

    char *base64Data = reinterpret_cast<char *>(heap_caps_malloc(kRxBase64BufSize, MALLOC_CAP_SPIRAM));
    assert(base64Data);
    REF_DEFER(heap_caps_free(base64Data));

    while (ctx->started) {
      const int rxBytes = uart_read_bytes(USED_UART_NUM, rawData, kRxBufSize - 1, CONFIG_KESPR_UARTD_READ_PERIOD / portTICK_PERIOD_MS);
      if (rxBytes <= 0) {
        continue;
      }

      base64_encode(rawData, rxBytes, base64Data);
      msg["data"] = base64Data;
      esp_err_t err = HttpD::BroadcastMsg(msg.as<JsonVariantConst>());
      if (err != ESP_OK) {
        ESP_LOGW(RX_TAG, "broadcast failed: %s", esp_err_to_name(err));
        continue;
      }
    }

    vTaskDelete(nullptr);
    ESP_LOGI(RX_TAG, "stopped");
  }
}

std::map<std::string, AppsMan::MsgHandler> UartApp::Handlers()
{
  return {
    {"tx", BIND_MSG_HANDLER(UartApp::HandleTx, this)}
  };
}

esp_err_t UartApp::Start()
{
  if (this->Started()) {
    return ESP_OK;
  }

  esp_err_t err = initUart();
  if (err != ESP_OK) {
    uart_driver_delete(USED_UART_NUM);
    ESP_LOGE(TAG, "init uart: %s", esp_err_to_name(err));
    return err;
  }

  this->txBuf = reinterpret_cast<uint8_t *>(heap_caps_malloc(kTxBufSize, MALLOC_CAP_SPIRAM));
  if (this->txBuf == nullptr) {
    ESP_LOGE(TAG, "unable to allocate tx buf");
    return ESP_ERR_NO_MEM;
  }

  this->ctx_.started = true;
  xTaskCreate(
    rxTask,
    "uart_rx_task",
    CONFIG_KESPR_UARTD_RX_STACK_SIZE,
    &this->ctx_,
    configMAX_PRIORITIES - 1,
    nullptr
  );

  KESPR::GUI::ChangeApp(KESPR::GUI::App::UART);
  KESPR::GUI::ChangeAppState(KESPR::GUI::AppState::Active);
  return AppsMan::App::Start();
}

esp_err_t UartApp::Stop()
{
  if (!!this->Started()) {
    return ESP_OK;
  }

  heap_caps_free(this->txBuf);
  this->txBuf = nullptr;
  this->ctx_.started = false;
  esp_err_t err = uart_driver_delete(UART_NUM_2);
  ESP_RETURN_ON_ERROR(err, TAG, "deinit uart");

  return AppsMan::App::Stop();
}

esp_err_t UartApp::HandleTx(int sockfd, const JsonObjectConst& reqJson, JsonObject& rspJson)
{
  (void)(sockfd);

  std::string_view base64Data = reqJson["data"];
  if (base64Data.size() > CONFIG_KESPR_UARTD_BUF_SIZE) {
    ESP_LOGE(TAG, "trying to parse too big msg: %d > %d", base64Data.size(), CONFIG_KESPR_UARTD_BUF_SIZE);
    return ESP_ERR_NO_MEM;
  }

  size_t dataLen = base64_decode(base64Data.data(), base64Data.size(), this->txBuf);
  uart_write_bytes(USED_UART_NUM, this->txBuf, dataLen);
  return ESP_OK;
}
