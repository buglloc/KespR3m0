#include <stdio.h>
#include <esp_log.h>
#include <esp_task.h>

#include <kespr_board.h>
#include <kespr_display.h>
#include <kespr_gui.h>
#include <kespr_net.h>
#include <defer.h>
#include <httpd/server.h>
#include <appsman/manager.h>

#include "helpers.h"
#include "uart.h"
#include "usbkb.h"


using namespace KESPR;
namespace {
  static const char *TAG = "main";
  const TickType_t xWaitDelay = 100 / portTICK_PERIOD_MS;

  static UartApp uartApp_;
  static UsbKbApp usbkbApp_;
}

extern "C"
{

void app_main(void)
{
#if CONFIG_KESPR_DEVMODE
  esp_log_level_set("*", ESP_LOG_DEBUG);
#else
  esp_log_level_set("*", ESP_LOG_INFO);
#endif

  ESP_LOGI(TAG, "initialize board");
  ESP_SHUTDOWN_ON_ERROR(Board::Initialize(), TAG, "initialize board");

  ESP_LOGI(TAG, "start display");
  ESP_SHUTDOWN_ON_ERROR(Display::Initialize(), TAG, "start display");

  ESP_LOGI(TAG, "start GUI");
  ESP_SHUTDOWN_ON_ERROR(GUI::Initialize(), TAG, "start GUI");

  ESP_LOGI(TAG, "start network");
  ESP_SHUTDOWN_ON_ERROR(Net::Start(), TAG, "start network");

  ESP_LOGI(TAG, "register apps");
  ESP_SHUTDOWN_ON_ERROR(AppsMan::Manager::Initialize(), TAG, "start app manager");
  ESP_SHUTDOWN_ON_ERROR(AppsMan::Manager::RegisterApp(&uartApp_), TAG, "register UART app");
  ESP_SHUTDOWN_ON_ERROR(AppsMan::Manager::RegisterApp(&usbkbApp_), TAG, "register USBKb app");

  ESP_LOGI(TAG, "start httpd service");
  ESP_SHUTDOWN_ON_ERROR(HttpD::Start(AppsMan::Manager::HandleRequest), TAG, "start HTTP server");

  ESP_LOGI(TAG, "started");
  const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;
  for (;;) {
    taskYIELD();
    vTaskDelay(xDelay);
  }
}

}
