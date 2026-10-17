#pragma once

#include "config.h"
#include "core/math/lpf.h"

// =============================================================================
// LAYER 4 SENSOR PREPROCESSING TASK RUNNERS (Calibration Correction + LPF)
// =============================================================================
void accel_gyro_preprocess_task(void* arg);
void mag_preprocess_task(void* arg);
