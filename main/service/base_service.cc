#include "base_service.h"
#include <esp_log.h>


using namespace Service;

namespace {
  static const char *TAG = "base_svc";
}

std::string BaseService::Name() {
  return this->name_;
};

bool BaseService::Started() {
  return this->started_;
};

esp_err_t BaseService::Initialize(httpd_handle_t server) {
  ESP_LOGI(TAG, "initialized service: %s", name_.c_str());
  this->httpd_ = server;
  return ESP_OK;
}

esp_err_t BaseService::Start() {
  ESP_LOGI(TAG, "started service: %s", name_.c_str());
  this->started_ = true;
  return ESP_OK;
}

esp_err_t BaseService::Stop() {
  ESP_LOGI(TAG, "stoped service: %s", name_.c_str());
  this->started_ = false;
  return ESP_OK;
}
