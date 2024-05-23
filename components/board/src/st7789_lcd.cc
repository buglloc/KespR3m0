#include "st7789_lcd.h"

#include <esp_check.h>


using namespace KESPR::Display;

esp_err_t ST7789LCD::Initialize()
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

    cfg.pin_sclk    =              36;
    cfg.pin_mosi    =              35;
    cfg.pin_miso    =              37;
    cfg.pin_dc      =              39;

    this->bus_.config(cfg);
    this->panel_.setBus(&this->bus_);
  }

  {
    auto cfg = this->panel_.config();

    cfg.pin_cs           =     7;
    cfg.pin_rst          =    40;
    cfg.pin_busy         =    -1;

    cfg.panel_width      =   135;
    cfg.panel_height     =   240;
    cfg.offset_x         =     0;
    cfg.offset_y         =     0;
    cfg.offset_rotation  =   3|4;
    cfg.dummy_read_pixel =     8;
    cfg.dummy_read_bits  =     1;
    cfg.readable         =  true;
    cfg.invert           =  true;
    cfg.rgb_order        = false;
    cfg.dlen_16bit       = false;
    cfg.bus_shared       =  true;

    this->panel_.config(cfg);
  }

  {
    auto cfg = backlight_.config();

    cfg.pin_bl          =    45;
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
