#include "core/safety/preflight_checks.h"
#include <Arduino.h>
#include <cmath>
#include "config.h"
#include "middleware/ipc.h"

// =============================================================================
// PREFLIGHT SAFETY GATES
// =============================================================================

SafetyStatus check_sensor_data_stream(){
  static SafetyStatus s_status = SafetyStatus::PENDING;
  if(s_status == SafetyStatus::FAILED) return SafetyStatus::FAILED;

  static uint32_t stream_check_start_ms = 0;
  static uint32_t healthy_start_ms = 0;

  uint32_t now_ms = millis();
  if(stream_check_start_ms == 0) stream_check_start_ms = now_ms;

  // 1. Buffer Occupancy Verification: Ensure all DAQ and preprocessor tasks have published data
  bool imu_raw_has_data  = !accel_raw.empty() && !gyro_raw.empty() && !raw_accel_gyro_timestamp.empty();
  bool mag_raw_has_data  = !mag_raw.empty() && !raw_mag_timestamp.empty();
  bool baro_raw_has_data = !pressure.empty() && !pressure_timestamp.empty();
  bool imu_filt_has_data = !accel.empty() && !gyro.empty() && !filtered_accel_gyro_timestamp.empty();
  bool mag_filt_has_data = !mag.empty() && !filtered_mag_timestamp.empty();

  bool has_data = imu_raw_has_data && mag_raw_has_data && baro_raw_has_data &&
                  imu_filt_has_data && mag_filt_has_data;

  uint32_t now_us = static_cast<uint32_t>(micros());

  // 2. Stream Latency & Freshness Verification (Microsecond resolution)
  bool imu_raw_fresh  = false;
  bool imu_filt_fresh = false;
  bool mag_raw_fresh  = false;
  bool mag_filt_fresh = false;
  bool baro_raw_fresh = false;

  if(imu_raw_has_data)
    imu_raw_fresh = ((now_us - raw_accel_gyro_timestamp.front()) < SENSOR_STREAM_IMU_TIMEOUT_US);
  if(imu_filt_has_data)
    imu_filt_fresh = ((now_us - filtered_accel_gyro_timestamp.front()) < SENSOR_STREAM_IMU_TIMEOUT_US);
  if(mag_raw_has_data)
    mag_raw_fresh = ((now_us - raw_mag_timestamp.front()) < SENSOR_STREAM_MAG_TIMEOUT_US);
  if(mag_filt_has_data)
    mag_filt_fresh = ((now_us - filtered_mag_timestamp.front()) < SENSOR_STREAM_MAG_TIMEOUT_US);
  if(baro_raw_has_data)
    baro_raw_fresh = ((now_us - pressure_timestamp.front()) < SENSOR_STREAM_BARO_TIMEOUT_US);

  bool fresh = imu_raw_fresh && imu_filt_fresh && mag_raw_fresh && mag_filt_fresh && baro_raw_fresh;

  // 3. Numerical Integrity & Physical Range Verification
  bool imu_valid  = false;
  bool mag_valid  = false;
  bool baro_valid = false;

  Vec3<float> a_sample{0.0f, 0.0f, 0.0f};
  Vec3<float> g_sample{0.0f, 0.0f, 0.0f};
  Vec3<float> m_sample{0.0f, 0.0f, 0.0f};
  float       p_sample = 0.0f;

  if(imu_raw_has_data){
    a_sample = accel_raw.front();
    g_sample = gyro_raw.front();
    float a_norm = a_sample.norm();
    float g_norm = g_sample.norm();
    imu_valid = std::isfinite(a_sample.x) && std::isfinite(a_sample.y) && std::isfinite(a_sample.z) &&
                std::isfinite(g_sample.x) && std::isfinite(g_sample.y) && std::isfinite(g_sample.z) &&
                (a_norm > 0.1f && a_norm < 3.0f) && (g_norm < 200.0f);
  }

  if(mag_raw_has_data){
    m_sample = mag_raw.front();
    float m_norm = m_sample.norm();
    mag_valid = std::isfinite(m_sample.x) && std::isfinite(m_sample.y) && std::isfinite(m_sample.z) &&
                (m_norm > 0.05f && m_norm < 5.0f);
  }

  if(baro_raw_has_data){
    p_sample = pressure.front();
    baro_valid = std::isfinite(p_sample) &&
                 (p_sample >= SENSOR_STREAM_BARO_MIN_PA && p_sample <= SENSOR_STREAM_BARO_MAX_PA);
  }

  bool valid = imu_valid && mag_valid && baro_valid;
  bool stream_healthy = has_data && fresh && valid;

  if(stream_healthy){
    if(healthy_start_ms == 0)
      healthy_start_ms = now_ms;
    else if((now_ms - healthy_start_ms) >= SENSOR_STREAM_HEALTHY_TIME_MS){
      if(s_status != SafetyStatus::PASSED){
        s_status = SafetyStatus::PASSED;
        Serial.println("[SAFETY] [GATE 1/6] Sensor data stream check PASSED! All streams active & healthy.");
      }
      return SafetyStatus::PASSED;
    }
  }else{
    healthy_start_ms = 0;

    // If stream was previously healthy and suddenly dropped
    if(s_status == SafetyStatus::PASSED){
      s_status = SafetyStatus::FAILED;
      Serial.println("[SAFETY] [GATE 1/6] Sensor data stream DROPPED while in PREFLIGHT!");
      return SafetyStatus::FAILED;
    }

    // Startup timeout check: If sensors failed to produce healthy streams within grace period
    if((now_ms - stream_check_start_ms) >= SENSOR_STREAM_STARTUP_TIMEOUT_MS){
      s_status = SafetyStatus::FAILED;
      Serial.println("[SAFETY] [GATE 1/6] Sensor data stream check TIMED OUT / FAILED!");
      if(!has_data)
        Serial.printf("      Missing data: IMU_raw:%d, MAG_raw:%d, BARO_raw:%d, IMU_filt:%d, MAG_filt:%d\n",
                      imu_raw_has_data, mag_raw_has_data, baro_raw_has_data, imu_filt_has_data, mag_filt_has_data);
      if(!fresh)
        Serial.printf("      Stale streams: IMU_raw:%d, IMU_filt:%d, MAG_raw:%d, MAG_filt:%d, BARO_raw:%d\n",
                      imu_raw_fresh, imu_filt_fresh, mag_raw_fresh, mag_filt_fresh, baro_raw_fresh);
      if(!valid)
        Serial.printf("      Invalid ranges: IMU:%d, MAG:%d, BARO:%d (a:%.2f, m:%.2f, p:%.1f)\n",
                      imu_valid, mag_valid, baro_valid, a_sample.norm(), m_sample.norm(), p_sample);
      return SafetyStatus::FAILED;
    }
  }

  return s_status;
}

SafetyStatus check_stationary_condition(){
  static SafetyStatus s_status = SafetyStatus::PENDING;
  if(s_status != SafetyStatus::PENDING) return s_status;

  static Vec3<float> last_raw_accel{0.0f, 0.0f, 0.0f};
  static Vec3<float> last_raw_gyro{0.0f, 0.0f, 0.0f};
  static uint32_t stationary_start_ms = 0;
  static uint32_t check_start_ms = 0;

  uint32_t current_time_ms = millis();
  if(check_start_ms == 0) check_start_ms = current_time_ms;

  Vec3<float> current_raw_accel = accel_raw.front();
  Vec3<float> current_raw_gyro  = gyro_raw.front();

  Vec3<float> delta_gyro  = current_raw_gyro - last_raw_gyro;
  Vec3<float> delta_accel = current_raw_accel - last_raw_accel;

  last_raw_gyro  = current_raw_gyro;
  last_raw_accel = current_raw_accel;

  bool gyro_motion_low  = (fabs(delta_gyro.x) < STATIONARY_GYRO_VAR_DPS) &&
                          (fabs(delta_gyro.y) < STATIONARY_GYRO_VAR_DPS) &&
                          (fabs(delta_gyro.z) < STATIONARY_GYRO_VAR_DPS);
  bool accel_motion_low = (fabs(delta_accel.x) < STATIONARY_ACCEL_VAR_MSS) &&
                          (fabs(delta_accel.y) < STATIONARY_ACCEL_VAR_MSS) &&
                          (fabs(delta_accel.z) < STATIONARY_ACCEL_VAR_MSS);

  if(gyro_motion_low && accel_motion_low){
    if(stationary_start_ms == 0)
      stationary_start_ms = current_time_ms;
    else if((current_time_ms - stationary_start_ms) >= STATIONARY_TIME_REQUIRED_MS){
      s_status = SafetyStatus::PASSED;
      Serial.println("[SAFETY] [GATE 2/6] Stationary condition check PASSED!");
      return s_status;
    }
  }else{
    stationary_start_ms = 0;

    if((current_time_ms - check_start_ms) >= PREFLIGHT_STATIONARY_TIMEOUT_MS){
      s_status = SafetyStatus::FAILED;
      Serial.printf("[SAFETY] [GATE 2/6] Stationary condition check FAILED! (Motion detected or settling timed out after %lu ms).\n",
                    static_cast<unsigned long>(PREFLIGHT_STATIONARY_TIMEOUT_MS));
      return s_status;
    }
  }

  return s_status;
}

SafetyStatus check_passive_sensor_calibration(bool is_stationary){
  static SafetyStatus s_status = SafetyStatus::PENDING;
  if(s_status != SafetyStatus::PENDING) return s_status;

  // Only evaluate when drone is verified stationary
  if(!is_stationary) return SafetyStatus::PENDING;

  Vec3<float> filt_gyro  = gyro.front();
  Vec3<float> filt_accel = accel.front();
  Vec3<float> filt_mag   = mag.front();

  bool gyro_bias_ok = (fabs(filt_gyro.x) < GYRO_PASSIVE_TOLERANCE_DPS) &&
                      (fabs(filt_gyro.y) < GYRO_PASSIVE_TOLERANCE_DPS) &&
                      (fabs(filt_gyro.z) < GYRO_PASSIVE_TOLERANCE_DPS);

  float accel_norm_g = filt_accel.norm();
  float accel_dev_mss = fabs(accel_norm_g - 1.0f) * 9.80665f;
  bool accel_bias_ok = (accel_dev_mss < ACCEL_PASSIVE_TOLERANCE_MSS);

  float mag_norm = filt_mag.norm();
  bool mag_norm_ok = (mag_norm > 0.1f && mag_norm < 2.0f);

  if(gyro_bias_ok && accel_bias_ok && mag_norm_ok){
    s_status = SafetyStatus::PASSED;
    Serial.println("[SAFETY] [GATE 3/6] Passive calibration check PASSED!");
  }else{
    s_status = SafetyStatus::FAILED;
    Serial.printf("[SAFETY] [GATE 3/6] Passive calibration check FAILED! (gyro_ok:%d, accel_dev:%.2fm/s^2, mag_norm:%.2f)\n",
                  gyro_bias_ok, accel_dev_mss, mag_norm);
  }

  return s_status;
}

SafetyStatus check_platform_tilt(){
  static SafetyStatus s_status = SafetyStatus::PENDING;
  if(s_status != SafetyStatus::PENDING) return s_status;

  if(accel.empty()) return SafetyStatus::PENDING;
  Vec3<float> a = accel.front();
  float a_norm = a.norm();
  if(!std::isfinite(a_norm) || a_norm < 0.5f || a_norm > 2.0f){
    s_status = SafetyStatus::FAILED;
    Serial.printf("[SAFETY] [GATE 4/6] Platform tilt check FAILED! Invalid accel norm: %.2f g\n", a_norm);
    return s_status;
  }

  // Calculate tilt angle theta relative to gravity Z-axis: cos(theta) = az / |a|
  float cos_tilt = a.z / a_norm;
  if(cos_tilt > 1.0f) cos_tilt = 1.0f;
  if(cos_tilt < -1.0f) cos_tilt = -1.0f;
  float tilt_deg = acosf(cos_tilt) * (180.0f / 3.14159265f);

  if(tilt_deg <= PREFLIGHT_MAX_TILT_DEG){
    s_status = SafetyStatus::PASSED;
    Serial.printf("[SAFETY] [GATE 4/6] Platform tilt check PASSED! (Incline: %.1f deg <= %.1f deg)\n",
                  tilt_deg, PREFLIGHT_MAX_TILT_DEG);
  }else{
    s_status = SafetyStatus::FAILED;
    Serial.printf("[SAFETY] [GATE 4/6] Platform tilt check FAILED! (Incline: %.1f deg > %.1f deg)\n",
                  tilt_deg, PREFLIGHT_MAX_TILT_DEG);
  }

  return s_status;
}

SafetyStatus check_power_initial_condition(){
  static SafetyStatus s_status = SafetyStatus::PENDING;
  if(s_status != SafetyStatus::PENDING) return s_status;

  static uint32_t power_check_start_ms = 0;
  uint32_t now_ms = millis();
  if(power_check_start_ms == 0) power_check_start_ms = now_ms;

  if(power_data.empty()){
    if((now_ms - power_check_start_ms) >= SENSOR_STREAM_STARTUP_TIMEOUT_MS){
      s_status = SafetyStatus::FAILED;
      Serial.println("[SAFETY] [GATE 5/6] Battery check TIMED OUT / FAILED! (No power module or battery detected). Transitioning to DEVELOPMENT.");
      return s_status;
    }
    return SafetyStatus::PENDING;
  }

  PowerMeasurement p = power_data.front();
  float v_mv = p.voltage_mv;

  // Numerical validity check
  if(!std::isfinite(v_mv) || v_mv < 1000.0f || v_mv > 36000.0f ||
     !std::isfinite(p.current_ma) || !std::isfinite(p.power_mw)){
    s_status = SafetyStatus::FAILED;
    Serial.printf("[SAFETY] [GATE 5/6] Power module initial check FAILED! Invalid reading: %.1f mV. Transitioning to DEVELOPMENT.\n", v_mv);
    return s_status;
  }

  // Auto-detect cell count & calculate per-cell voltage
  int32_t cell_count = static_cast<int32_t>(roundf(v_mv / 3800.0f));
  if(cell_count < 1) cell_count = 1;
  if(cell_count > 8) cell_count = 8;
  float cell_v_mv = v_mv / static_cast<float>(cell_count);

  if(cell_v_mv < BATTERY_CELL_MIN_VOLTAGE_MV){
    s_status = SafetyStatus::FAILED;
    Serial.printf("[SAFETY] [GATE 5/6] Battery initial check FAILED! Pack: %.1f mV (%.1f mV/cell on %dS < %.1f mV/cell). Transitioning to DEVELOPMENT.\n",
                  v_mv, cell_v_mv, static_cast<int>(cell_count), BATTERY_CELL_MIN_VOLTAGE_MV);
    return s_status;
  }

  s_status = SafetyStatus::PASSED;
  Serial.printf("[SAFETY] [GATE 5/6] Power module & battery initial check PASSED! Pack: %.1f mV (%.1f mV/cell on %dS)\n",
                v_mv, cell_v_mv, static_cast<int>(cell_count));
  return s_status;
}

SafetyStatus check_rc_controller_signal(){
  static SafetyStatus s_status = SafetyStatus::PENDING;
  if(s_status != SafetyStatus::PENDING) return s_status;

  static uint32_t rc_check_start_ms = 0;
  uint32_t now_ms = millis();
  if(rc_check_start_ms == 0) rc_check_start_ms = now_ms;

  if(rc_data.empty()){
    if((now_ms - rc_check_start_ms) >= SENSOR_STREAM_STARTUP_TIMEOUT_MS){
      s_status = SafetyStatus::FAILED;
      Serial.println("[SAFETY] [GATE 6/6] RC receiver signal check TIMED OUT / FAILED! (No RC stream detected). Transitioning to DEVELOPMENT.");
      return s_status;
    }
    return SafetyStatus::PENDING;
  }

  const RcChannels& ch = rc_data.front();
  if(ch.failsafe){
    s_status = SafetyStatus::FAILED;
    Serial.println("[SAFETY] [GATE 6/6] RC receiver signal check FAILED! (Failsafe active). Transitioning to DEVELOPMENT.");
    return s_status;
  }

  s_status = SafetyStatus::PASSED;
  Serial.printf("[SAFETY] [GATE 6/6] RC receiver signal check PASSED! (Signal locked, active channels: %u)\n",
                ch.channel_count);
  return s_status;
}

SafetyStatus check_storage_health(){
  // [FUTURE CHECK] Verify SD card sector capacity and mount status
  return SafetyStatus::PASSED;
}
