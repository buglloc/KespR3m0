#pragma once
#include <sdkconfig.h>

#include <stdint.h>

#include <esp_err.h>
#include <esp_log.h>


#define _STRINGIFY(x) #x
#define STRINGIFY(s) _STRINGIFY(s)

#define ASSERT_ACTION abort();

#define ESP_SHUTDOWN_ON_ERROR(x, log_tag, format, ...) do {                                             \
  esp_err_t err_rc_ = (x);                                                                              \
  if (unlikely(err_rc_ != ESP_OK)) {                                                                    \
    ESP_LOGE(log_tag, "[%d] %s(%d): " format, (int)err_rc_, __FUNCTION__, __LINE__, ##__VA_ARGS__);     \
    ASSERT_ACTION                                                                                       \
  }                                                                                                     \
} while(0)

#define SHUTDOWN_ON_ERROR(x, log_tag, format, ...) do {                                              \
  Error err_ = (x);                                                                                  \
  if (unlikely(err_ != Error::None)) {                                                               \
    ESP_LOGE(log_tag, "[%d] %s(%d): " format, (int)err_, __FUNCTION__, __LINE__, ##__VA_ARGS__);     \
    ASSERT_ACTION                                                                                    \
  }                                                                                                  \
} while(0)

#define HALT_ASSERT(x, log_tag, format, ...) do {                                       \
  if (unlikely(!(x))) {                                                                 \
    ESP_LOGE(log_tag, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__);        \
    ASSERT_ACTION                                                                       \
  }                                                                                     \
} while(0)
