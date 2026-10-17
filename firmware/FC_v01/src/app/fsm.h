#pragma once
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// =============================================================================
// SYSTEM FLIGHT STATES
// =============================================================================
enum SystemState : uint16_t {
  // Normal expected flight states
  UNINIT = 0,
  CALIB,
  PREFLIGHT,
  READY,
  TAKEOFF,
  FLIGHT,
  LANDING,    // [TEMPORARY NOTE] Currently bypassed; direct FLIGHT -> POSTFLIGHT transition on 3s hold
  POSTFLIGHT,
  DEVELOPMENT,

  // Failure / Emergency flight states
  LEVELOUT,
  CRASHED,
  FREEFALL
};

// =============================================================================
// SYSTEM EVENT MASKS (EVT_ prefix)
// =============================================================================
enum SystemEventMasks : uint32_t {
  EVT_NONE                      = 0,

  // Calibration & Button Events
  EVT_ACTIVE_CALIB_INVOKED      = (1 << 0),  // User held button at startup/trigger
  EVT_CALIB_COMPLETE            = (1 << 1),  // Active calibration routine finished
  EVT_CALIB_FAILED              = (1 << 2),  // Active calibration routine aborted/failed
  EVT_BTN_SHORT_CLICK           = (1 << 3),  // User button short press (< 500 ms)
  EVT_BTN_LONG_HOLD             = (1 << 4),  // User button held for >= 3000 ms

  // Pre-Flight Check 1: Stationary Detection
  EVT_STATIONARY_MET            = (1 << 5),  // Gyro & accel variance within bounds for required duration
  EVT_STATIONARY_FAIL           = (1 << 16), // Stationary settling timed out or excessive motion

  // Pre-Flight Check 2: Passive Sensor Calibration Check
  EVT_PASSIVE_CALIB_PASS        = (1 << 6),  // Stationary sensor offsets within tolerance
  EVT_PASSIVE_CALIB_FAIL        = (1 << 7),  // Stationary sensor offsets out of tolerance (requires recalibration)

  // [FUTURE CHECK] Pre-Flight Check 3: Platform Tilt Check
  // Evaluates accelerometer gravity vector angle relative to level ground plane
  EVT_TILT_CHECK_PASS           = (1 << 8),
  EVT_TILT_CHECK_FAIL           = (1 << 9),

  // Pre-Flight Check 4: Sensor Data Stream & Hardware Bus Health
  // Checks fresh raw and filtered sensor data streaming rates and numerical validity
  EVT_SENSOR_STREAM_OK          = (1 << 10),
  EVT_SENSOR_STREAM_FAIL        = (1 << 11),

  // [FUTURE CHECK] Pre-Flight Check 5: RC Controller Link & Failsafe
  // Checks receiver signal loss, packet frame validity, and arm switch position
  EVT_RC_SIGNAL_OK              = (1 << 12),
  EVT_RC_SIGNAL_FAIL            = (1 << 13),

  // [FUTURE CHECK] Pre-Flight Check 6: Storage / Blackbox Health
  // Checks SD card presence, SPI flash mount, and logging capacity
  EVT_STORAGE_OK                = (1 << 14),
  EVT_STORAGE_FAIL              = (1 << 15),

  // Pre-Flight Check: Battery Voltage & Power Health
  EVT_BATTERY_VOLTAGE_OK        = (1 << 17),
  EVT_BATTERY_LOW               = (1 << 18),

  // Emergency / Fail-Safe Events
  EVT_EMERGENCY_SHUTDOWN        = (1 << 19)
};

// =============================================================================
// TRANSITION RULE MATCH POLICY
// =============================================================================
enum MatchPolicy : uint8_t {
  ALL = 0,
  ANY,
  ALWAYS,
  NEVER
};

// =============================================================================
// TRANSITION RULE STRUCTURE
// =============================================================================
struct TransitRule {
  SystemState from_state;
  SystemState to_state;

  uint32_t trigger_mask;
  uint32_t neg_trigger_mask;
  MatchPolicy trigger_policy;

  uint32_t blocker_mask;
  MatchPolicy block_policy;

  const char* rule_name;
};

// =============================================================================
// FSM PUBLIC API
// =============================================================================
void fsm_init();
void fsm_task(void* arg);
SystemState fsm_get_state();
uint32_t fsm_get_events();
void fsm_set_event(uint32_t event_mask);
void fsm_clear_event(uint32_t event_mask);
void fsm_trigger_emergency();
bool fsm_is_armed_allowed();
bool sensor_data_stream_check();
bool check_stationary_condition();
bool check_passive_sensor_calibration();
bool check_platform_tilt();
bool check_power_initial_condition();
bool check_rc_controller_signal();
