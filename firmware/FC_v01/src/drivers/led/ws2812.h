#pragma once
#include <cstdint>
#include "platforms/hal/hal_pulse.h"

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

enum PaletteColorId : uint8_t {
  PALETTE_OFF = 0,
  PALETTE_RED,
  PALETTE_GREEN,
  PALETTE_BLUE,       // Barometer
  PALETTE_CYAN,       // Gyroscope
  PALETTE_MAGENTA,    // Magnetometer
  PALETTE_YELLOW,
  PALETTE_ORANGE,     // Accelerometer
  PALETTE_PURPLE,
  PALETTE_LIME,
  PALETTE_TEAL,
  PALETTE_PINK,
  PALETTE_AMBER,
  PALETTE_WHITE,
  PALETTE_COUNT
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
  explicit WS2812(HAL_PulseTX& tx) : pulse_tx(tx) {}

  bool init(int gpio_pin);
  void set_rgb(uint8_t r, uint8_t g, uint8_t b);
  void set_brightness(uint8_t scale); // 0 (off) to 255 (max)
  void set_high();
  void set_low();
};
