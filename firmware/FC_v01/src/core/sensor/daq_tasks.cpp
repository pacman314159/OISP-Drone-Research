#include <Arduino.h>
#include "daq_tasks.h"
#include "config.h"
#include <esp_timer.h>
#include "middleware/ipc.h"
#include "middleware/sysview_tracing.h"
#include "drivers/imu/mpu6050.h"
#include "drivers/imu/hmc5883l.h"
#include "drivers/baro/bmp180.h"
#include "drivers/power/ina226.h"
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
        raw_accel_gyro_timestamp.push(ts_us);
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
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id); //=========================================================================
    
    uint32_t ts_us = static_cast<uint32_t>(micros());

    if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
      bool success = hmc->get_all_data(mag);
      hmc->set_meas_mode(HMC_MODE_SINGLE);
      xSemaphoreGive(i2c0_mutex);

      if(success){
        mag_raw.push(mag);
        raw_mag_timestamp.push(ts_us);
      }
    }

    SYSVIEW_END(task_id); //=========================================================================
  }
}

void baro_daq_task(void* arg){
  if(arg == nullptr) return;

  BMP180* bmp = static_cast<BMP180*>(arg);
  const uint32_t task_id = TASK_BARO_DAQ_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_BARO_DAQ_FREQ_HZ);
  const BmpOss oss = static_cast<BmpOss>(BMP180_DEFAULT_OSS);
  bmp->set_oversampling(oss);

  // Pre-loop initial temperature reading and first pressure trigger
  if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
    bmp->start_temperature_meas();
    xSemaphoreGive(i2c0_mutex);
  }
  vTaskDelay(pdMS_TO_TICKS(bmp->get_temp_conversion_delay_ms()));

  if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
    int32_t init_ut = 0;
    bmp->read_temperature_raw(init_ut);
    bmp->start_pressure_meas(oss);
    xSemaphoreGive(i2c0_mutex);
  }

  uint32_t cycle_count = 0;
  const uint32_t temp_refresh_interval = TASK_BARO_DAQ_FREQ_HZ; // 1 Hz temperature refresh
  bool waiting_for_temp = false;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id); //=========================================================================

    uint32_t ts_us = static_cast<uint32_t>(micros());

    if(waiting_for_temp){
      // Temperature conversion completed during the inter-loop period
      if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
        int32_t ut = 0;
        bmp->read_temperature_raw(ut);
        bmp->start_pressure_meas(oss);
        xSemaphoreGive(i2c0_mutex);
      }
      waiting_for_temp = false;
    }else{
      // Pressure conversion completed during the inter-loop period
      int32_t up = 0;
      bool success = false;
      if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
        success = bmp->read_pressure_raw(up);
        cycle_count++;

        if(cycle_count >= temp_refresh_interval){
          cycle_count = 0;
          waiting_for_temp = true;
          bmp->start_temperature_meas();
        }else{
          bmp->start_pressure_meas(oss);
        }
        xSemaphoreGive(i2c0_mutex);
      }

      if(success){
        float pressure_pa = bmp->calculate_pressure(up, oss);
        pressure.push(pressure_pa);
        pressure_timestamp.push(ts_us);
      }
    }

    SYSVIEW_END(task_id); //=========================================================================
  }
}

void power_daq_task(void* arg){
  if(arg == nullptr) return;

  INA226* ina = static_cast<INA226*>(arg);
  const uint32_t task_id = TASK_POWER_DAQ_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_POWER_DAQ_FREQ_HZ);

  if(ina->is_initialized()){
    Serial.println("[BOOT] INA226 Power Module initialized successfully on I2C1.");
  }else{
    Serial.println("[BOOT] INA226 Power Module not detected on I2C1 (Bench / Development Mode).");
  }

  float accumulated_energy_mah = 0.0f; // mAh
  uint32_t last_calc_ts_us = static_cast<uint32_t>(micros());
  uint32_t consecutive_failures = 0;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id); //=========================================================================

    if(ina->is_initialized()){
      uint32_t ts_us = static_cast<uint32_t>(micros());
      PowerSample sample = {};
      bool success = false;

      if(xSemaphoreTake(i2c1_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
        success = ina->read_sample(sample);
        if(!success){
          consecutive_failures++;
          // If consecutive failures occur, attempt fast recovery for contact bounce (50-150ms),
          // then back off to slow cadence (every 3s) to avoid bus contention
          if(consecutive_failures >= 5 && consecutive_failures <= 15 && (consecutive_failures % 5) == 0){
            if(ina->init())
              consecutive_failures = 0;
          }else if(consecutive_failures > 15 && (consecutive_failures % (TASK_POWER_DAQ_FREQ_HZ * 3)) == 0){
            if(ina->init())
              consecutive_failures = 0;
          }
        }else{
          consecutive_failures = 0;
        }
        xSemaphoreGive(i2c1_mutex);
      }

      if(success){
        float dt_s = static_cast<float>(ts_us - last_calc_ts_us) * 1e-6f;
        if(dt_s > 0.0f && dt_s < 1.0f)
          accumulated_energy_mah += (sample.current_ma * dt_s) / 3600.0f;
        last_calc_ts_us = ts_us;

        PowerMeasurement meas = {
          sample.voltage_mv,
          sample.current_ma,
          sample.power_mw,
          accumulated_energy_mah
        };

        power_data.push(meas);
        power_timestamp.push(ts_us);
      }
    }

    SYSVIEW_END(task_id); //=========================================================================
  }
}
