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
    IMUSample sample;
    bool success = mpu->get_all_data(sample);
    xSemaphoreGive(i2c0_mutex);

    if(success) imu_ring_buffer.push(sample);
    Serial.printf("%.4f, %.4f, %.4f\n", sample.a[0], sample.a[1], sample.a[2]);
  }
}

void mag_daq_task(void* arg){

}

void baro_daq_task(void* arg){

}

