#include <Arduino.h>
#include "daq_tasks.h"
#include "config.h"
#include "esp_timer.h"
#include "middleware/ipc.h"
#include "middleware/sysview_tracing.h"
#include "drivers/imu/mpu6050.h"
#include "drivers/imu/hmc5883l.h"
#include "platforms/hal/hal_simd.h"

void accel_gyro_daq_task(void* arg){
  if(arg == nullptr) return;

  MPU6050* mpu = static_cast<MPU6050*>(arg);
  const uint32_t task_id = TASK_ACCEL_GYRO_DAQ_ID;

  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_ACCEL_GYRO_DAQ_FREQ_HZ);

  Vec3<float> accel, gyro;
  float temp = 0.0f;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2))
      last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id); //=========================================================================

    uint32_t ts_us = static_cast<uint32_t>(micros());

    if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
      bool success = mpu->get_all_data(accel, gyro, temp);
      xSemaphoreGive(i2c0_mutex);

      if(success){
        accel_raw.push(accel);
        gyro_raw.push(gyro);
        accel_gyro_timestamp.push(ts_us);
      }
    }

    SYSVIEW_END(task_id); //=========================================================================
  }
}

void mag_daq_task(void* arg){
  if(arg == nullptr) return;

  HMC5883L* hmc = static_cast<HMC5883L*>(arg);
  const uint32_t task_id = TASK_MAG_DAQ_ID;

  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_MAG_DAQ_FREQ_HZ);

  Vec3<float> mag;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2))
      last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id); //=========================================================================

    if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
      bool success = hmc->get_all_data(mag);
      hmc->set_meas_mode(HMC_MODE_SINGLE);
      xSemaphoreGive(i2c0_mutex);

      if(success) mag_raw.push(mag);
    }

    SYSVIEW_END(task_id); //=========================================================================
  }
}

void baro_daq_task(void* arg){}
