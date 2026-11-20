#pragma once
#include <cstdint>

// =============================================================================
// SYSTEM EVENT MASKS (EVT_ prefix)  —  Layer 3 contract
// =============================================================================
// Layer 4 modules report events through these masks instead of importing
// app/fsm.h (Layer 5). Reporting is not deciding: Layer 5 (fsm.cpp) owns the
// event set and every transition decision (0046, controlled inversion).
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

  // Emergency / Fail-Safe Events
  EVT_EMERGENCY_SHUTDOWN        = (1 << 19),

  // Takeoff & In-Flight Transition Events
  EVT_TAKEOFF_INITIATED         = (1 << 20), // Pilot advanced throttle or toggled arm switch to initiate takeoff
  EVT_TAKEOFF_COMPLETE          = (1 << 21), // 5-second takeoff window satisfied with all safety guards intact
  EVT_TAKEOFF_ABORT             = (1 << 22), // Pilot throttled down / disarmed back to ready
  EVT_TAKEOFF_SAFETY_VIOLATION  = (1 << 23)  // Runaway rate (>150 dps), excessive tilt (>30 deg), or heading divergence
};

// Implemented by Layer 5 (app/fsm.cpp). Lower layers may only report or clear
// events here; they never read or decide state (0042, 0046).
void fsm_set_event(uint32_t event_mask);
void fsm_clear_event(uint32_t event_mask);
