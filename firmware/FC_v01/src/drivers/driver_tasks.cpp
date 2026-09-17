#include <Arduino.h>
#include "driver_tasks.h"
#include "config.h"
#include "middleware/ipc.h"
#include "drivers/imu/mpu6050.h"

void accel_gyro_daq_task(void* arg){
  if(arg == nullptr) return;

  MPU6050* mpu = static_cast<MPU6050*>(arg);
  TickType_t last_wake_time = xTaskGetTickCount();
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_IMU_DAQ_FREQ_HZ);

  while(true){
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) != pdTRUE) continue;
    Vec3<int16_t> accel_raw;
    Vec3<int16_t> gyro_raw;
    float temp_raw = 0.0f;
    bool success = mpu->get_all_data_raw(accel_raw, gyro_raw, temp_raw);
    xSemaphoreGive(i2c0_mutex);

    if(success){
      accel_raw_msb.push(accel_raw);
      gyro_raw_msb.push(gyro_raw);
    }
    Serial.printf("%d, %d, %d\n", accel_raw.x, accel_raw.y, accel_raw.z);
  }
}

void mag_daq_task(void* arg){

}

void baro_daq_task(void* arg){

}

