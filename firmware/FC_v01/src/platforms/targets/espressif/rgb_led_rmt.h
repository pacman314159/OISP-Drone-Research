#pragma once
#include <cstdint>
#include "platforms/hal/hal_rgb_led.h"

class RGB_LED_RMT : public HAL_RGB_LED {
public:
  RGB_LED_RMT();
  ~RGB_LED_RMT() override = default;

  void init(uint8_t gpio_pin) override;
  void set_pixel(uint8_t r, uint8_t g, uint8_t b) override;
  void show() override;

private:
  uint8_t _gpio_pin;
  uint8_t _r;
  uint8_t _g;
  uint8_t _b;
  bool _initialized;
  void* _rmt_channel_handle;
};
