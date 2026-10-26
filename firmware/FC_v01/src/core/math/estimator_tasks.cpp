#include "estimator_tasks.h"
#include <Arduino.h>
#include <cmath>
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/critical_section.h"
#include "middleware/sysview_tracing.h"

// Global EKF Instances
AttitudeEKF g_ekf;
AltitudeEKF g_alt_ekf;

// =============================================================================
// LAYER 4 FREERTOS TASK RUNNERS (DUAL-TASK ARCHITECTURE - OPTION B)
// =============================================================================

// High-Speed Pilot Task (500 Hz): Gyro Integration & Gravity Accel Update
void ekf_roll_pitch_task(void* arg){
  const uint32_t task_id = TASK_EKF_ROLL_PITCH_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_EKF_ROLL_PITCH_FREQ_HZ);

  uint32_t last_ts_us = 0;
  TickType_t last_wake_time = xTaskGetTickCount();

  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(accel.empty() || gyro.empty() || filtered_accel_gyro_timestamp.empty()) continue;

    SYSVIEW_START(task_id); //=========================================================================

    Vec3<float> a_sample = accel.front();
    Vec3<float> g_sample = gyro.front();
    uint32_t ts_us = filtered_accel_gyro_timestamp.front();

    float dt = (last_ts_us > 0) ? (static_cast<float>(ts_us - last_ts_us) * 1e-6f) : (1.0f / static_cast<float>(TASK_EKF_ROLL_PITCH_FREQ_HZ));
    last_ts_us = ts_us;
    if(dt <= 0.0f || dt > 0.05f) dt = 1.0f / static_cast<float>(TASK_EKF_ROLL_PITCH_FREQ_HZ);

    if(xSemaphoreTake(ekf_state_mutex, pdMS_TO_TICKS(5)) == pdTRUE){
      g_ekf.predict(dt, g_sample);
      g_ekf.update_accel(a_sample);

      Vec4 q = g_ekf.get_quaternion();
      Vec3<float> euler = g_ekf.get_euler_angles();

      xSemaphoreGive(ekf_state_mutex);

      att_quat.push(q);
      att_euler.push(euler);
      att_timestamp.push(ts_us);
    }

    SYSVIEW_END(task_id); //=========================================================================
  }
}

// Low-Speed Navigator Task (100 Hz): Heading Magnetometer Update
void ekf_yaw_task(void* arg){
  const uint32_t task_id = TASK_EKF_YAW_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_EKF_YAW_FREQ_HZ);

  uint32_t last_mag_ts_us = 0;
  TickType_t last_wake_time = xTaskGetTickCount();

  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(mag.empty() || filtered_mag_timestamp.empty()) continue;

    uint32_t ts_us = filtered_mag_timestamp.front();
    if(ts_us == last_mag_ts_us) continue; // Skip redundant updates if no new sample

    SYSVIEW_START(task_id); //=========================================================================

    Vec3<float> m_sample = mag.front();
    last_mag_ts_us = ts_us;

    if(xSemaphoreTake(ekf_state_mutex, pdMS_TO_TICKS(5)) == pdTRUE){
      g_ekf.update_mag(m_sample);
      xSemaphoreGive(ekf_state_mutex);
    }

    SYSVIEW_END(task_id); //=========================================================================
  }
}

// Medium-Speed Vertical Velocity & Altitude Task (100 Hz): Accel + Quat Predict, Baro Update
void ekf_altitude_task(void* arg){
  const uint32_t task_id = TASK_EKF_ALTITUDE_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_EKF_ALTITUDE_FREQ_HZ);

  uint32_t last_predict_ts_us = 0;
  uint32_t last_baro_ts_us = 0;
  TickType_t last_wake_time = xTaskGetTickCount();

  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(accel.empty() || att_quat.empty() || filtered_accel_gyro_timestamp.empty()) continue;

    SYSVIEW_START(task_id); //=========================================================================

    Vec3<float> a_sample = accel.front();
    Vec4 q_sample = att_quat.front();
    uint32_t ts_us = filtered_accel_gyro_timestamp.front();

    float dt = (last_predict_ts_us > 0) ? (static_cast<float>(ts_us - last_predict_ts_us) * 1e-6f) : (1.0f / static_cast<float>(TASK_EKF_ALTITUDE_FREQ_HZ));
    last_predict_ts_us = ts_us;
    if(dt <= 0.0f || dt > 0.05f) dt = 1.0f / static_cast<float>(TASK_EKF_ALTITUDE_FREQ_HZ);

    if(xSemaphoreTake(ekf_state_mutex, pdMS_TO_TICKS(5)) == pdTRUE){
      // 1. Predict Step (100 Hz): Quaternion-projected inertial acceleration
      g_alt_ekf.predict(dt, a_sample, q_sample);

      // 2. Barometer Update Step (50 Hz): Non-blocking check for fresh pressure reading
      if(!pressure.empty() && !pressure_timestamp.empty()){
        uint32_t baro_ts = pressure_timestamp.front();
        if(baro_ts != last_baro_ts_us){
          last_baro_ts_us = baro_ts;
          float p = pressure.front();
          g_alt_ekf.update_baro(p);
        }
      }

      Vec3<float> output = g_alt_ekf.get_output_vector();
      xSemaphoreGive(ekf_state_mutex);

      // 3. Publish to Layer 3 IPC ring buffers
      alt_state.push(output);
      alt_timestamp.push(ts_us);
    }

    SYSVIEW_END(task_id); //=========================================================================
  }
}
