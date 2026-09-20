#include <Arduino.h>
#include "daq_tasks.h"
#include "config.h"
#include "middleware/ipc.h"
#include "drivers/imu/mpu6050.h"
#include "platforms/hal/hal_simd.h"
#include "platforms/hal/hal_rgb_led.h"
#include "drivers/led/ws2812b.h"

void accel_gyro_daq_task(void* arg){
  if(arg == nullptr) return;

  MPU6050* mpu = static_cast<MPU6050*>(arg);
  TickType_t last_wake_time = xTaskGetTickCount();
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_IMU_DAQ_FREQ_HZ);

  Vec3<float> accel, gyro;
  float temp = 0.0f;

  while(true){
    vTaskDelayUntil(&last_wake_time, period_ticks);

    uint32_t ts_us = static_cast<uint32_t>(micros());

    if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) != pdTRUE) continue;
    bool success = mpu->get_all_data(accel, gyro, temp);
    xSemaphoreGive(i2c0_mutex);

    if(success){
      accel_raw.push(accel);
      gyro_raw.push(gyro);
      accel_gyro_timestamp.push(ts_us);
    }
  }
}

void mag_daq_task(void* arg){}

void baro_daq_task(void* arg){}

void led_blink_task(void* arg){
  WS2812B led(get_rgb_led());
  led.init(RGB_LED_PIN);

  TickType_t last_wake_time = xTaskGetTickCount();
  const TickType_t blink_period_ticks = pdMS_TO_TICKS(1000 / TASK_LED_BLINK_FREQ_HZ);
  bool led_state = false;

  while(true){
    led_state = !led_state;

    if(led_state){
      led.set_color(0, 255, 0);
    }else{
      led.off();
    }

    vTaskDelayUntil(&last_wake_time, blink_period_ticks);
  }
}
