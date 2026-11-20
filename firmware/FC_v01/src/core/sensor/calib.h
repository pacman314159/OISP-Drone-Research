#pragma once
#include <cstdint>
#include "core/math/matrix.h"
#include "drivers/storage/flash_storage_driver.h"
#include "ipc_types.h"   // CalibTarget / CalibStatus / CalibReportMsg contract (C15)

// =============================================================================
// SENSOR CALIBRATION DOMAIN TYPES & PROTOCOL STRUCTURES
// =============================================================================

// Sensor Calibration Shared State
struct SensorCalibrationData {
  Vec3<float>         gyro_bias{0.0f, 0.0f, 0.0f};   // Gyroscope zero-rate bias offsets (deg/s)
  Vec3<float>         accel_bias{0.0f, 0.0f, 0.0f};  // Accelerometer zero-g bias offset v0 (g)
  Vec3<float>         accel_scale{1.0f, 1.0f, 1.0f}; // Accelerometer 3-axis sensitivity scaling s (g^-1)
  Vec3<float>         mag_bias{0.0f, 0.0f, 0.0f};    // Magnetometer hard-iron bias offset v0 (Gauss)
  Matrix<float, 3, 3> mag_soft_iron{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}; // Magnetometer soft-iron matrix W * B0
  float               mag_field_ref{1.0f};           // Geomagnetic reference field B0 (Gauss)
  float               baro_ref_pa{101325.0f};        // Ground reference pressure P0 (Pa)
  bool                is_valid{false};
};

extern SensorCalibrationData calib_data;
extern FlashStorageDriver*   calib_storage;

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
