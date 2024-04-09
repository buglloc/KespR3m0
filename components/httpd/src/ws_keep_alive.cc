#include "ws_keep_alive.h"
#include <esp_timer.h>
#include <esp_err.h>

#include "ws_utils.h"


using namespace HttpD;

namespace {
  static const char *TAG = "httpd::ws::keepalive";

  static uint64_t _tick_get_ms(void)
  {
    return esp_timer_get_time()/1000;
  }

  static void keepAliveBgTask(void* arg)
  {
    KeepAlive::Context *ctx = static_cast<KeepAlive::Context *>(arg);

    KeepAlive::Action action;
    bool stopped = false;
    while (!stopped) {
      auto haveAction = xQueueReceive(ctx->q, (void *) &action, ctx->ToNextCheck() / portTICK_PERIOD_MS);
      if (haveAction == pdTRUE) {
        switch (action.kind) {
        case KeepAlive::ActionKind::AddClient: {
          esp_err_t err = ctx->AddClient(action.fd);
          if (err != ESP_OK) {
            ESP_LOGE(TAG, "add client fail (fd=%d): %s", action.fd, esp_err_to_name(err));
          }
          break;
        }

        case KeepAlive::ActionKind::TouchClient: {
          esp_err_t err = ctx->TouchClient(action.fd, action.lastSeen);
          if (err != ESP_OK) {
            ESP_LOGE(TAG, "touch client fail (fd=%d): %s", action.fd, esp_err_to_name(err));
          }
          break;
        }

        case KeepAlive::ActionKind::RemoveClient: {
          esp_err_t err = ctx->RemoveClient(action.fd);
          if (err != ESP_OK) {
            ESP_LOGE(TAG, "remove client fail (fd=%d): %s", action.fd, esp_err_to_name(err));
          }
          break;
        }

        case KeepAlive::ActionKind::Stop:
          stopped = true;
          break;

        default:
          ESP_LOGE(TAG, "unexpected action: %d", static_cast<uint8_t>(action.kind));
          break;
        }

        continue;
      }

      // timeout: check if PING message needed
      ctx->PingClients();
    }

    vQueueDelete(ctx->q);
    vTaskDelete(NULL);
  };
};

TickType_t KeepAlive::Context::ToNextCheck()
{
  int64_t toNextCheck = 30000;
  uint64_t tickMs = _tick_get_ms();
  for (const auto &client: this->clients) {
      if (client.state != KeepAlive::ClientState::Active) {
        continue;
      }

      uint64_t checkClientAt = client.lastSeen + CONFIG_KESPR_WS_CHECK_ALIVE_PERIOD;
      if (checkClientAt >= toNextCheck + tickMs) {
        continue;
      }

      toNextCheck = checkClientAt - tickMs;
      if (toNextCheck < 0) {
        toNextCheck = 1000;
      }
  }

  return toNextCheck;
}

esp_err_t KeepAlive::Context::AddClient(int sockfd)
{
  for (auto &client: this->clients) {
    if (client.state != KeepAlive::ClientState::None) {
      continue;
    }

    client.state = KeepAlive::ClientState::Active;
    client.fd = sockfd;
    client.lastSeen = _tick_get_ms();
    return ESP_OK;
  }

  return ESP_ERR_NO_MEM;
}

esp_err_t KeepAlive::Context::TouchClient(int sockfd, uint64_t lastSeen)
{
  for (auto &client: this->clients) {
    if (client.state != KeepAlive::ClientState::Active) {
      continue;
    }

    if (client.fd != sockfd) {
      continue;
    }

    client.lastSeen = lastSeen;
    return ESP_OK;
  }

  return ESP_ERR_NOT_FOUND;
}

esp_err_t KeepAlive::Context::RemoveClient(int sockfd)
{
  for (auto &client: this->clients) {
    if (client.state != KeepAlive::ClientState::Active) {
      continue;
    }

    if (client.fd != sockfd) {
      continue;
    }

    client.state = KeepAlive::ClientState::None;
    client.fd = -1;
    client.lastSeen = 0;
    return ESP_OK;
  }

  return ESP_OK;
}

void KeepAlive::Context::PingClients()
{
  uint64_t tickMs = _tick_get_ms();
  for (const auto &client: this->clients) {
    if (client.state != KeepAlive::ClientState::Active) {
      continue;
    }

    if (client.lastSeen + CONFIG_KESPR_WS_CHECK_ALIVE_PERIOD > tickMs) {
      continue;
    }

    auto socketType = httpd_ws_get_fd_info(this->httpd_, client.fd);
    if (socketType != HTTPD_WS_CLIENT_WEBSOCKET) {
      ESP_LOGD(TAG, "client is not a ws, forget it (fd=%d)", client.fd);
      this->RemoveClient(client.fd);
      continue;
    }

    ESP_LOGD(TAG, "haven't seen the client for a while (fd=%d)", client.fd);
    if (client.lastSeen + CONFIG_KESPR_WS_INACTIVE_TIMEOUT <= tickMs) {
      ESP_LOGW(TAG, "client not alive, closing it (fd=%d)", client.fd);
      esp_err_t err = httpd_sess_trigger_close(this->httpd_, client.fd);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "unable to close client session (fd=%d): %s", client.fd, esp_err_to_name(err));
      }
      continue;
    }

    esp_err_t err = Ws::SendAsyncFrame(this->httpd_, client.fd, [](httpd_ws_frame_t *frame, void *arg) {
      (void)(arg);
      frame->payload = NULL;
      frame->len = 0;
      frame->type = HTTPD_WS_TYPE_PING;
    });
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "unable to ping client (fd=%d): %s", client.fd, esp_err_to_name(err));
    }
  }
}

esp_err_t KeepAliveManager::Initialize(httpd_handle_t server)
{
  if (this->initialized_) {
    ESP_LOGE(TAG, "keep alive manager already initialized");
    return ESP_ERR_INVALID_STATE;
  }

  ESP_LOGI(TAG, "initializing keep-alive manager");
  this->ctx.q = xQueueCreate(CONFIG_KESPR_HTTP_MAX_CLIENTS/2, sizeof(KeepAlive::Action));
  this->ctx.clients.resize(CONFIG_KESPR_HTTP_MAX_CLIENTS);
  this->ctx.httpd_ = server;
  this->initialized_ = true;
  return ESP_OK;
}

esp_err_t KeepAliveManager::Start()
{
  if (!this->initialized_) {
    ESP_LOGE(TAG, "keep alive manager is not initialized");
    return ESP_ERR_INVALID_STATE;
  }

  ESP_LOGI(TAG, "starts keep-alive manager");
  auto started = xTaskCreate(
    keepAliveBgTask,
    "keep_alive_bg_task",
    CONFIG_KESPR_WS_TASK_STACK_SIZE,
    &this->ctx,
    tskIDLE_PRIORITY+1,
    nullptr
  );
  if (started != pdTRUE) {
    ESP_LOGE(TAG, "bg task start failed");
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t KeepAliveManager::Stop()
{
  if (!this->initialized_) {
    ESP_LOGE(TAG, "keep alive manager is not initialized");
    return ESP_ERR_INVALID_STATE;
  }

  ESP_LOGI(TAG, "stops keep-alive manager");
  KeepAlive::Action action = {
    .kind = KeepAlive::ActionKind::Stop
  };
  xQueueSendToBack(this->ctx.q, &action, 0);
  return ESP_OK;
}

esp_err_t KeepAliveManager::AddClient(int sockfd)
{
  KeepAlive::Action action = {
    .kind = KeepAlive::ActionKind::AddClient,
    .fd = sockfd
  };
  if (xQueueSendToBack(this->ctx.q, &action, 0) == pdTRUE) {
      return ESP_OK;
  }

  return ESP_FAIL;
}

esp_err_t KeepAliveManager::RemoveClient(int sockfd)
{
  KeepAlive::Action action = {
    .kind = KeepAlive::ActionKind::RemoveClient,
    .fd = sockfd
  };
  if (xQueueSendToBack(this->ctx.q, &action, 0) == pdTRUE) {
      return ESP_OK;
  }

  return ESP_FAIL;
}

esp_err_t KeepAliveManager::TouchClient(int sockfd)
{
  KeepAlive::Action action = {
    .kind = KeepAlive::ActionKind::TouchClient,
    .fd = sockfd,
    .lastSeen = _tick_get_ms()
  };
  if (xQueueSendToBack(this->ctx.q, &action, 0) == pdTRUE) {
      return ESP_OK;
  }

  return ESP_FAIL;
}

KeepAliveManager* KeepAliveManager::Global(httpd_handle_t server)
{
  assert(server != nullptr);
  return static_cast<HttpD::KeepAliveManager*>(httpd_get_global_user_ctx(server));
}
