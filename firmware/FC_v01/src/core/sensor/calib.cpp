#include "core/sensor/calib.h"
#include <cstring>
#include <cstdio>
#include "middleware/ipc.h"
#include "config.h"
#include "platforms/hal/hal_simd.h"

static constexpr const char* CALIB_NAMESPACE = "sensor_calib";
static constexpr const char* CALIB_KEY       = "calib_data";

void calib_reset_defaults(){
  calib_data.gyro_bias     = Vec3<float>(0.0f, 0.0f, 0.0f);
  calib_data.accel_bias    = Vec3<float>(0.0f, 0.0f, 0.0f);
  calib_data.accel_scale   = Vec3<float>(1.0f, 1.0f, 1.0f);
  calib_data.mag_bias      = Vec3<float>(0.0f, 0.0f, 0.0f);
  calib_data.mag_soft_iron = Matrix<float, 3, 3>::identity();
  calib_data.mag_field_ref = 1.0f;
  calib_data.is_valid      = false;
}

bool calib_init(FlashStorageDriver* storage){
  if(storage != nullptr) calib_storage = storage;
  calib_reset_defaults();

  if(calib_storage != nullptr)
    return calib_load_from_flash();

  return false;
}

bool calib_load_from_flash(){
  if(calib_storage == nullptr) return false;

  bool loaded = false;
  if(xSemaphoreTake(flash_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
    SensorCalibrationData temp;
    if(calib_storage->read_record(CALIB_NAMESPACE, CALIB_KEY, &temp, sizeof(SensorCalibrationData))){
      if(temp.is_valid){
        calib_data = temp;
        loaded = true;
      }
    }
    xSemaphoreGive(flash_mutex);
  }
  return loaded;
}

bool calib_save_to_nvs(){
  if(calib_storage == nullptr) return false;

  calib_data.is_valid = true;

  bool saved = false;
  if(xSemaphoreTake(flash_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
    saved = calib_storage->write_record(CALIB_NAMESPACE, CALIB_KEY, &calib_data, sizeof(SensorCalibrationData));
    xSemaphoreGive(flash_mutex);
  }
  return saved;
}

SensorCalibrationData& calib_get_data(){
  return calib_data;
}

void calib_apply_gyro(Vec3<float>& raw_gyro){
  raw_gyro -= calib_data.gyro_bias;
}

void calib_apply_accel(Vec3<float>& raw_accel){
  raw_accel = (raw_accel - calib_data.accel_bias).hadamard(calib_data.accel_scale);
}

void calib_apply_mag(Vec3<float>& raw_mag){
  raw_mag = calib_data.mag_soft_iron * (raw_mag - calib_data.mag_bias);
}
