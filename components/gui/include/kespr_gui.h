#pragma once
#include <string>

#include <esp_err.h>


namespace KESPR::GUI
{
  enum class App : int32_t
  {
    None,
    USBKb,
    UART,
  };

  enum class AppState : int32_t
  {
    Inactive,
    PartialLeft,
    PartialRight,
    Active,
  };

  void ChangeApp(const App app);
  void ChangeAppState(const AppState state);
  esp_err_t Initialize();
}
