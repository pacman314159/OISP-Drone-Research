#pragma once
#include <cstdint>
#include "core/math/matrix.h"
#include "drivers/storage/flash_storage_driver.h"

// =============================================================================
// SENSOR CALIBRATION DOMAIN TYPES & PROTOCOL STRUCTURES
// =============================================================================

// Calibration Subsystem Reporting Structures (Worker Producer, FSM Consumer)
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
