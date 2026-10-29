#include <Arduino.h>
#include "core/sensor/sensor_preprocess_tasks.h"
#include "core/sensor/calib.h"
#include "core/math/lpf.h"
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/critical_section.h"
#include "middleware/sysview_tracing.h"

#include "drivers/storage/flash_storage_driver.h"

void accel_gyro_preprocess_task(void* arg){
  FlashStorageDriver* flash = static_cast<FlashStorageDriver*>(arg);
  const uint32_t task_id = TASK_ACCEL_GYRO_PREPROCESS_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_ACCEL_GYRO_PREPROCESS_FREQ_HZ);

  if(flash != nullptr && flash->init("nvs")){
    if(calib_init(flash)){
      const SensorCalibrationData& cal = calib_get_data();
      Serial.printf("[PREPROCESS] Calibration loaded: Gyro Bias [%.3f, %.3f, %.3f], Accel Bias [%.3f, %.3f, %.3f], Accel Scale [%.3f, %.3f, %.3f], Mag Bias [%.3f, %.3f, %.3f], Mag B0: %.3f G, Baro P0: %.2f Pa\n",
                    cal.gyro_bias.x, cal.gyro_bias.y, cal.gyro_bias.z,
                    cal.accel_bias.x, cal.accel_bias.y, cal.accel_bias.z,
                    cal.accel_scale.x, cal.accel_scale.y, cal.accel_scale.z,
                    cal.mag_bias.x, cal.mag_bias.y, cal.mag_bias.z,
                    cal.mag_field_ref, cal.baro_ref_pa);
    }else{
      Serial.println("[PREPROCESS] No valid calibration found in Flash! Preprocess output suspended.");
    }
  }else{
    Serial.println("[PREPROCESS] Flash NVS initialization failed! Preprocess output suspended.");
  }

  LowPassFilter1stOrder accel_lpf;
  LowPassFilter2ndOrder gyro_lpf;

  lpf_init(&accel_lpf, static_cast<float>(TASK_ACCEL_GYRO_PREPROCESS_FREQ_HZ), ACCEL_LPF_CUTOFF_FREQ_HZ);
  gyro_lpf.init(static_cast<float>(TASK_ACCEL_GYRO_PREPROCESS_FREQ_HZ), GYRO_LPF_CUTOFF_FREQ_HZ, GYRO_LPF_ZETA);

  Vec3<float> a_raw;
  Vec3<float> g_raw;
  uint32_t ts_us = 0;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(!calib_get_data().is_valid) continue;

    SYSVIEW_START(task_id); //=========================================================================

    a_raw = accel_raw.front();
    g_raw = gyro_raw.front();
    ts_us = raw_accel_gyro_timestamp.front();

    calib_apply_accel(a_raw);
    calib_apply_gyro(g_raw);

    Vec3<float> a_clean = lpf_apply(&accel_lpf, a_raw);
    Vec3<float> g_clean = gyro_lpf.apply(g_raw);

    accel.push(a_clean);
    gyro.push(g_clean);
    filtered_accel_gyro_timestamp.push(ts_us);

    SYSVIEW_END(task_id); //=========================================================================
  }
}

void mag_preprocess_task(void* arg){
  const uint32_t task_id = TASK_MAG_PREPROCESS_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_MAG_PREPROCESS_FREQ_HZ);

  LowPassFilter1stOrder mag_lpf;
  lpf_init(&mag_lpf, static_cast<float>(TASK_MAG_PREPROCESS_FREQ_HZ), MAG_LPF_CUTOFF_FREQ_HZ);

  Vec3<float> m_raw;
  uint32_t ts_us = 0;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(!calib_get_data().is_valid) continue;

    SYSVIEW_START(task_id); //=========================================================================

    m_raw = mag_raw.front();
    ts_us = raw_mag_timestamp.front();

    calib_apply_mag(m_raw);

    Vec3<float> m_clean = lpf_apply(&mag_lpf, m_raw);
    m_clean.normalize(); // Ensure output is strictly on a unit sphere for Extended Kalman Filter

    mag.push(m_clean);
    filtered_mag_timestamp.push(ts_us);

    SYSVIEW_END(task_id); //=========================================================================
  }
}
