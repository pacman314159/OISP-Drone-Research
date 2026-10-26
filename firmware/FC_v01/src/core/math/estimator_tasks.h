#pragma once
#include "estimators.h"
#include "config.h"

// =============================================================================
// LAYER 4 FLIGHT CORE & ALGORITHMS - STATE ESTIMATOR TASK RUNNERS
// =============================================================================

// Global EKF Instances
extern AttitudeEKF g_ekf;
extern AltitudeEKF g_alt_ekf;

// Layer 4 FreeRTOS Task Runners
void ekf_roll_pitch_task(void* arg);
void ekf_yaw_task(void* arg);
void ekf_altitude_task(void* arg);
