#include "display_wshare_lcd.h"

#include <esp_check.h>


using namespace KESPR::Display;

esp_err_t WShareLCD::Initialize()
{
  {
    auto cfg = this->bus_.config();
    
    cfg.spi_host    =       SPI2_HOST;
    cfg.spi_mode    =               0;
    cfg.freq_write  =        80000000;
    cfg.freq_read   =        80000000;
    cfg.spi_3wire   =            true;
    cfg.use_lock    =            true;
    cfg.dma_channel = SPI_DMA_CH_AUTO;

    cfg.pin_sclk    =              10;
    cfg.pin_mosi    =              11;
    cfg.pin_miso    =              12;
    cfg.pin_dc      =               8;

    this->bus_.config(cfg);
    this->panel_.setBus(&this->bus_);
  }

  {
    auto cfg = this->panel_.config();

    cfg.pin_cs           =     9;
    cfg.pin_rst          =    14;
    cfg.pin_busy         =    -1;

    cfg.panel_width      =   240;
    cfg.panel_height     =   240;
    cfg.offset_x         =     0;
    cfg.offset_y         =     0;
    cfg.offset_rotation  =     0;
    cfg.dummy_read_pixel =     8;
    cfg.dummy_read_bits  =     1;
    cfg.readable         =  true;
    cfg.invert           = true;
    cfg.rgb_order        = false;
    cfg.dlen_16bit       = false;
    cfg.bus_shared       =  true;

    this->panel_.config(cfg);
  }

  {
    auto cfg = backlight_.config();

    cfg.pin_bl          =     2;
    cfg.invert          = false;
    cfg.freq            = 44100;
    cfg.pwm_channel     =     7;

    this->backlight_.config(cfg);
    this->panel_.setLight(&this->backlight_);
  }

  this->setBrightness(0);
  this->setPanel(&this->panel_);
  if (!this->init_without_reset()) {
    return ESP_FAIL;
  }

  return ESP_OK;
}
