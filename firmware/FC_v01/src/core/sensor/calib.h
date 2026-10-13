#pragma once
#include <cstdint>
#include "core/math/matrix.h"
#include "middleware/ipc.h"
#include "drivers/storage/flash_storage_driver.h"

// =============================================================================
// SENSOR CALIBRATION INTERFACE
// =============================================================================
bool calib_init(FlashStorageDriver* storage = nullptr);
bool calib_load_from_flash();
bool calib_save_to_nvs();
void calib_reset_defaults();
SensorCalibrationData& calib_get_data();

// Hardware SIMD-Accelerated Vector Calibration Corrections
void calib_apply_gyro(Vec3<float>& raw_gyro);
void calib_apply_accel(Vec3<float>& raw_accel);
void calib_apply_mag(Vec3<float>& raw_mag);
