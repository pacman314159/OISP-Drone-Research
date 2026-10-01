#include <Arduino.h>
#include "src/core/filters/filter_tasks.h"
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/critical_section.h"
#include "middleware/sysview_tracing.h"

void accel_gyro_lpf_task(void* arg){
  const uint32_t task_id = TASK_ACCEL_GYRO_LPF_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_ACCEL_GYRO_LPF_FREQ_HZ);

  LowPassFilter1stOrder accel_lpf;
  LowPassFilter1stOrder gyro_lpf;

  lpf_init(&accel_lpf, static_cast<float>(TASK_ACCEL_GYRO_LPF_FREQ_HZ), ACCEL_LPF_CUTOFF_FREQ_HZ);
  lpf_init(&gyro_lpf, static_cast<float>(TASK_ACCEL_GYRO_LPF_FREQ_HZ), GYRO_LPF_CUTOFF_FREQ_HZ);

  Vec3<float> a_raw;
  Vec3<float> g_raw;
  uint32_t ts_us = 0;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id); //=========================================================================

    a_raw = accel_raw.front();
    g_raw  = gyro_raw.front();
    ts_us = raw_accel_gyro_timestamp.front();

    Vec3<float> a_clean = lpf_apply(&accel_lpf, a_raw);
    Vec3<float> g_clean = lpf_apply(&gyro_lpf, g_raw);

    SYS_ENTER_CRITICAL();
    accel = a_clean;
    gyro  = g_clean;
    accel_gyro_timestamp = ts_us;
    SYS_EXIT_CRITICAL();

    accel_filtered.push(a_clean);
    gyro_filtered.push(g_clean);
    filtered_accel_gyro_timestamp.push(ts_us);

    SYSVIEW_END(task_id); //=========================================================================
  }
}

void mag_lpf_task(void* arg){
  const uint32_t task_id = TASK_MAG_LPF_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_MAG_LPF_FREQ_HZ);

  LowPassFilter1stOrder mag_lpf;
  lpf_init(&mag_lpf, static_cast<float>(TASK_MAG_LPF_FREQ_HZ), MAG_LPF_CUTOFF_FREQ_HZ);

  Vec3<float> m_raw;
  uint32_t ts_us = 0;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id); //=========================================================================

    m_raw = mag_raw.front();
    ts_us = raw_mag_timestamp.front();

    Vec3<float> m_clean = lpf_apply(&mag_lpf, m_raw);

    SYS_ENTER_CRITICAL();
    mag = m_clean;
    mag_timestamp = ts_us;
    SYS_EXIT_CRITICAL();

    mag_filtered.push(m_clean);
    filtered_mag_timestamp.push(ts_us);

    SYSVIEW_END(task_id); //=========================================================================
  }
}
