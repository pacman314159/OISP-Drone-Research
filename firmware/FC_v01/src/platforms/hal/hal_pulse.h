#pragma once
#include <cstdint>
#include <cstddef>

struct PulseSymbol {
  uint32_t duration0_ns; // Phase 0 duration in nanoseconds
  uint8_t  level0;       // Phase 0 logic level (0 or 1)
  uint32_t duration1_ns; // Phase 1 duration in nanoseconds
  uint8_t  level1;       // Phase 1 logic level (0 or 1)
};

class HAL_PulseTX {
public:
  virtual ~HAL_PulseTX() = default;
  virtual bool init(int gpio_pin, uint32_t clock_resolution_hz = 10000000) = 0;
  virtual bool write_symbols(const PulseSymbol* symbols, size_t count) = 0;
  virtual bool wait_tx_done(uint32_t timeout_ms = 10) = 0;
};

// Global Target Pulse Accessor (Pure HAL interface, defined per platform target build)
HAL_PulseTX& get_pulse_tx0();
