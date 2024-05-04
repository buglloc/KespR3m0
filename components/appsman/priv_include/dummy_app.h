#pragma once
#include <esp_err.h>

#include <ArduinoJson.h>
#include <appsman/app.h>


namespace AppsMan
{
  class DummyApp final : public AppsMan::App
  {
  public:
    DummyApp() : AppsMan::App("dummy") {};
  };
}
