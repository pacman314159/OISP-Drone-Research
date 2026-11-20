#pragma once
#include <cstdint>
#include "platforms/hal/hal_pulse.h"
#include "ipc_types.h"   // PaletteColorId / LedCommand contract (C15)

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

Color get_palette_color(PaletteColorId id);

class WS2812 {
private:
  HAL_PulseTX& pulse_tx;
  uint8_t      current_r{0};
  uint8_t      current_g{0};
  uint8_t      current_b{0};
  uint8_t      brightness{255}; // 0 = dark, 255 = 100% full scale
  bool         is_on{false};
  PulseSymbol  symbol_buffer[24]; // 24 GRB bits for single LED

  void encode_and_transmit(uint8_t r, uint8_t g, uint8_t b);

public:
  explicit WS2812(HAL_PulseTX& tx) : pulse_tx(tx){}

  bool init(int gpio_pin);
  void set_rgb(uint8_t r, uint8_t g, uint8_t b);
  void set_brightness(uint8_t scale); // 0 (off) to 255 (max)
  void set_high();
  void set_low();
};
