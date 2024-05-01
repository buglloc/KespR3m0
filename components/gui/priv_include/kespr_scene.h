#pragma once
#include "kespr_gui.h"

#include <esp_err.h>


namespace KESPR::GUI::Scene
{
  const int32_t kMaxLevel = 100;

  void SetApp(const KESPR::GUI::App app);
  void SetAppState(const KESPR::GUI::AppState state);
  void SetBattLevel(const int32_t level);
  void SetWiFiLevel(const int32_t level);

  esp_err_t Show();
}
