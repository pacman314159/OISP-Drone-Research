#pragma once
#include "platforms/hal/hal_pulse.h"
#include <driver/rmt.h>

class ESP32_PulseTX : public HAL_PulseTX {
private:
  rmt_channel_t channel{RMT_CHANNEL_1};
  bool          initialized{false};
  uint32_t      resolution_hz{10000000}; // 10 MHz (1 tick = 100 ns)

public:
  ESP32_PulseTX() = default;
  ~ESP32_PulseTX() override;

  bool init(int gpio_pin, uint32_t clock_resolution_hz = 10000000) override;
  bool write_symbols(const PulseSymbol* symbols, size_t count) override;
  bool wait_tx_done(uint32_t timeout_ms = 10) override;
};
