#pragma once
#include <esp_err.h>
#include <esp_netif.h>


namespace KESPR::Net
{
  enum class Mode: uint8_t
  {
    None = 0,
    STA,
    AP
  };

  esp_err_t Start();
  esp_ip4_addr_t CurrentIP();
  Mode CurrentMode();
}
