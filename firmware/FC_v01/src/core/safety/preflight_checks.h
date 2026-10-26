#pragma once
#include "core/safety/safety_types.h"

// =============================================================================
// PREFLIGHT SAFETY GATES
// =============================================================================
SafetyStatus check_sensor_data_stream();
SafetyStatus check_stationary_condition();
SafetyStatus check_passive_sensor_calibration(bool is_stationary);
SafetyStatus check_platform_tilt();
SafetyStatus check_power_initial_condition();
SafetyStatus check_rc_controller_signal();
SafetyStatus check_storage_health();
