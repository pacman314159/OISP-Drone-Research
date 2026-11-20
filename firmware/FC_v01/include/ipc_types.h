#pragma once
#include <cstdint>

// =============================================================================
// LAYER-NEUTRAL IPC CONTRACTS (POD ONLY)
// =============================================================================
// Types that cross an IPC boundary live here, below every consumer.
// Rule (0002 Rule 6 as rewritten by C15):
//   Any type that crosses an IPC boundary MUST be a POD declared in this
//   header. No OS type (FreeRTOS), no vendor type (esp_*), no template math.
// This header sits in include/ — the neutral zone that every layer may include
// without inverting the dependency direction.

// -----------------------------------------------------------------------------
// RC receiver contract (producer: Layer 2 FS_iA6B, consumer: Layers 3-5)
// -----------------------------------------------------------------------------
constexpr uint8_t  RC_MAX_CHANNELS         = 8;

struct RcChannels {
  uint16_t channels[RC_MAX_CHANNELS]; // Channel values in microseconds (1000..2000 us)
  uint8_t  channel_count;             // Number of valid decoded channels (typically 6)
  bool     failsafe;                  // True if signal lost / timed out
};

// -----------------------------------------------------------------------------
// Status LED contract (producer: Layer 5 FSM, consumer: Layer 4 interface task)
// -----------------------------------------------------------------------------
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

enum LedPattern : uint8_t {
  LED_PATTERN_OFF = 0,
  LED_PATTERN_SOLID,
  LED_PATTERN_BLINK_SLOW,
  LED_PATTERN_BLINK_FAST,
  LED_PATTERN_BREATHE,
  LED_PATTERN_FLASH_BURST
};

struct LedCommand {
  LedPattern     pattern;
  PaletteColorId color;
};

// -----------------------------------------------------------------------------
// Calibration report contract (producer: Layer 4 worker, consumer: Layer 5 FSM)
// -----------------------------------------------------------------------------
enum CalibTarget : uint8_t {
  CALIB_TARGET_NONE = 0,
  CALIB_TARGET_GYRO,
  CALIB_TARGET_ACCEL,
  CALIB_TARGET_MAG,
  CALIB_TARGET_BARO
};

enum CalibStatus : uint8_t {
  CALIB_STATUS_IDLE = 0,
  CALIB_STATUS_SETTLING,      // Pre-sampling hands-off settling phase
  CALIB_STATUS_SAMPLING,      // Active stationary data acquisition phase
  CALIB_STATUS_HEALTHCHECK,
  CALIB_STATUS_STEP_LOCKED,   // Interactive step confirmed
  CALIB_STATUS_COMPUTING,     // End-stage iterative solver running
  CALIB_STATUS_COMPLETED,     // Finished successfully, waiting for FSM deletion
  CALIB_STATUS_FAILED         // Failed/timed out, waiting for FSM deletion
};

struct CalibReportMsg {
  CalibTarget target;
  CalibStatus status;
  uint8_t     current_step;   // e.g. 0..5 for 6-point accel
  uint8_t     progress_pct;   // 0..100%
  uint32_t    timestamp_us;   // Healthcheck timestamp
};

// -----------------------------------------------------------------------------
// Actuator command contract (producer: Layer 5 FSM, consumer: Layer 4 motor)
// -----------------------------------------------------------------------------
enum MotorCommand : uint8_t {
  MOTOR_CMD_DISARM = 0,
  MOTOR_CMD_ARM,
  MOTOR_CMD_EMERGENCY_STOP
};

enum MotorNotificationBits : uint32_t {
  NOTIFY_MOTOR_NONE           = 0,
  NOTIFY_MOTOR_EMERGENCY_STOP = (1 << 0)
};
