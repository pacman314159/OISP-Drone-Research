#pragma once
#include <cstdint>
#include <cstddef>
#include "platforms/hal/hal_pulse.h"

constexpr uint8_t  RC_MAX_CHANNELS         = 8;
constexpr uint16_t PPM_MIN_CHANNEL_US      = 800;
constexpr uint16_t PPM_MAX_CHANNEL_US      = 2200;
constexpr uint16_t PPM_CENTER_CHANNEL_US   = 1500;
constexpr uint32_t PPM_FAILSAFE_TIMEOUT_US = 250000; // 250 ms signal loss threshold

struct RcChannels {
  uint16_t channels[RC_MAX_CHANNELS]; // Channel values in microseconds (1000..2000 us)
  uint8_t  channel_count;             // Number of valid decoded channels (typically 6)
  bool     failsafe;                  // True if signal lost / timed out
};

class FS_iA6B {
public:
  explicit FS_iA6B(HAL_PulseRX& pulse_rx);

  bool init(int gpio_pin);
  bool update(RcChannels& out_channels, uint32_t timeout_ms = 25);
  bool is_failsafe() const;

private:
  HAL_PulseRX& _pulse_rx;
  RcChannels   _last_channels;
  uint32_t     _last_frame_us;
};
