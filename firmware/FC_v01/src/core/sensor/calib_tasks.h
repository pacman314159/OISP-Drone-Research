#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// =============================================================================
// LAYER 4 SENSOR CALIBRATION TASK RUNNERS
// =============================================================================

// Singular Calibration Tasks (Fixed series averaging and storage)
void task_calib_gyro(void* arg);
void task_calib_baro(void* arg);

// Iterative Calibration Tasks (Human-in-the-loop acquisition + Ellipsoid fit solver)
void task_calib_accel(void* arg);
void task_calib_mag(void* arg);
