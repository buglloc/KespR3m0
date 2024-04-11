#pragma once
#include <vector>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_http_server.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>


namespace HttpD::KeepAlive
{
  enum class ActionKind : uint8_t {
    None = 0,
    AddClient,
    TouchClient,
    RemoveClient,
    Stop,
  };

  struct Action
  {
    ActionKind kind;
    int fd;
    uint64_t lastSeen;
  };

  enum class ClientState : uint8_t {
    None = 0,
    Active,
  };

  struct Client
  {
    ClientState state = ClientState::None;
    int fd = 0;
    uint64_t lastSeen = 0;
  };

  class Context
  {
  public:
    TickType_t ToNextCheck();
    esp_err_t AddClient(int sockfd);
    esp_err_t TouchClient(int sockfd, uint64_t lastSeen);
    esp_err_t RemoveClient(int sockfd);
    void PingClients();

  public:
    QueueHandle_t q = nullptr;
    std::vector<Client> clients = {};
    httpd_handle_t httpd_ = nullptr;
  };
}

namespace HttpD
{
  class KeepAliveManager
  {
  public:
    static KeepAliveManager* Global(httpd_handle_t server);

  public:
    esp_err_t Initialize(httpd_handle_t server);
    esp_err_t Start();
    esp_err_t Stop();
    esp_err_t AddClient(int sockfd);
    esp_err_t RemoveClient(int sockfd);
    esp_err_t TouchClient(int sockfd);

  protected:
    bool initialized_ = false;
    KeepAlive::Context ctx = {};
  };
}
