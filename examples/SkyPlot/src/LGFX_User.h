#pragma once
// ===========================================================================
//  YOUR DISPLAY GOES HERE
//
//  The SkyPlot example only needs an object called `LGFX_User` that is a LovyanGFX device. This file
//  is a starting point for a common case (ST7789 on SPI, ESP32 family): replace the panel
//  class, bus type, pins, size and offsets with the ones for your display. See
//  https://github.com/lovyan03/LovyanGFX for the other panels and buses (I2C, 8/16-bit
//  parallel, RGB, touch, backlight...).
//
//  Every value marked  <<< CHANGE  is a placeholder, not a recommendation.
// ===========================================================================
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX_User : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;		// <<< CHANGE: your panel driver (Panel_ILI9341, Panel_ST7735, Panel_GC9A01, ...)
  lgfx::Bus_SPI      _bus;			// <<< CHANGE: your bus type
  lgfx::Light_PWM    _light;		// backlight (remove if the backlight is not PWM controlled)

public:
  LGFX_User() {
    {	// bus
      auto cfg = _bus.config();
      cfg.spi_host   = SPI2_HOST;	// <<< CHANGE: SPI host of your chip
      cfg.spi_mode   = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false;
      cfg.use_lock   = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk   = 12;			// <<< CHANGE: pins
      cfg.pin_mosi   = 11;
      cfg.pin_miso   = -1;
      cfg.pin_dc     = 13;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {	// panel
      auto cfg = _panel.config();
      cfg.pin_cs           = 10;		// <<< CHANGE
      cfg.pin_rst          = 14;		// <<< CHANGE (-1 if not connected)
      cfg.pin_busy         = -1;
      cfg.panel_width      = 240;		// <<< CHANGE: native (unrotated) size
      cfg.panel_height     = 320;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 0;
      cfg.readable         = false;
      cfg.invert           = true;		// <<< CHANGE if the colours look inverted
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = false;
      _panel.config(cfg);
    }
    {	// backlight
      auto cfg = _light.config();
      cfg.pin_bl      = 15;			// <<< CHANGE (or delete this block and the Light_PWM member)
      cfg.invert      = false;
      cfg.freq        = 44100;
      cfg.pwm_channel = 7;
      _light.config(cfg);
      _panel.setLight(&_light);
    }
    setPanel(&_panel);
  }
};
