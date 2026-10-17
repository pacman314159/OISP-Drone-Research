#include "app/fsm.h"
#include <Arduino.h>
#include <cmath>
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/critical_section.h"
#include "middleware/sysview_tracing.h"
#include "drivers/led/ws2812.h"
#include "core/interface/user_button.h"
#include "core/sensor/calib_tasks.h"
#include "core/sensor/calib.h"

// =============================================================================
// INTERNAL FSM STATE & SYNCHRONIZATION
// =============================================================================
static volatile SystemState s_current_state = UNINIT;
static volatile uint32_t s_current_events = EVT_NONE;
static volatile bool s_events_dirty = true;

// Active Calibration Sub-State & Worker Synchronization
enum CalibSubState : uint8_t {
  CALIB_SUBSTATE_SELECT_MENU = 0,
  CALIB_SUBSTATE_MONITORING,
  CALIB_SUBSTATE_DONE,
  CALIB_SUBSTATE_FAILED
};

static TaskHandle_t s_active_calib_handle = nullptr;

static PaletteColorId get_sensor_menu_color(uint8_t idx){
  switch(idx){
    case 0:  return PALETTE_CYAN;    // Gyroscope
    case 1:  return PALETTE_ORANGE;  // Accelerometer
    case 2:  return PALETTE_MAGENTA; // Magnetometer
    case 3:  return PALETTE_BLUE;    // Barometer
    default: return PALETTE_WHITE;
  }
}

// =============================================================================
// TRANSITION RULES SPECIFICATION
// =============================================================================
static const TransitRule TRANSITION_RULES[] = {
  // 1. UNINIT -> PREFLIGHT (Normal boot sequence: direct to PREFLIGHT)
  {
    UNINIT,
    PREFLIGHT,
    EVT_NONE,
    EVT_NONE,
    MatchPolicy::ALWAYS,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Uninit_To_Preflight"
  },

  // 2. PREFLIGHT -> CALIB (User held button or invoked active calibration from PREFLIGHT)
  {
    PREFLIGHT,
    CALIB,
    EVT_ACTIVE_CALIB_INVOKED,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Preflight_To_Calib"
  },

  // 3. CALIB -> PREFLIGHT (Calibration finished successfully or aborted)
  {
    CALIB,
    PREFLIGHT,
    EVT_CALIB_COMPLETE,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN | EVT_CALIB_FAILED,
    MatchPolicy::ANY,
    "Rule_Calib_To_Preflight"
  },

  // 4. PREFLIGHT -> READY (All 6 Mandatory Safety Gates Passed; No Fail Blockers)
  {
    PREFLIGHT,
    READY,
    EVT_SENSOR_STREAM_OK | EVT_STATIONARY_MET | EVT_PASSIVE_CALIB_PASS |
    EVT_TILT_CHECK_PASS | EVT_BATTERY_VOLTAGE_OK | EVT_RC_SIGNAL_OK,
    EVT_NONE,
    MatchPolicy::ALL,
    EVT_EMERGENCY_SHUTDOWN | EVT_SENSOR_STREAM_FAIL | EVT_STATIONARY_FAIL |
    EVT_PASSIVE_CALIB_FAIL | EVT_TILT_CHECK_FAIL | EVT_BATTERY_LOW | EVT_RC_SIGNAL_FAIL,
    MatchPolicy::ANY,
    "Rule_Preflight_To_Ready"
  },

  // 5. PREFLIGHT -> DEVELOPMENT (Any Mandatory Safety Gate Failed)
  {
    PREFLIGHT,
    DEVELOPMENT,
    EVT_SENSOR_STREAM_FAIL | EVT_STATIONARY_FAIL | EVT_PASSIVE_CALIB_FAIL |
    EVT_TILT_CHECK_FAIL | EVT_BATTERY_LOW | EVT_RC_SIGNAL_FAIL,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_NONE,
    MatchPolicy::NEVER,
    "Rule_Preflight_To_Development"
  },

  // 6. READY -> FLIGHT (Arming / Takeoff command - future expansion)
  {
    READY,
    FLIGHT,
    EVT_NONE,
    EVT_NONE,
    MatchPolicy::NEVER,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Ready_To_Flight"
  },

  // 7. FLIGHT -> POSTFLIGHT (Temporary bypass around LANDING: 3s button hold terminates flight)
  {
    FLIGHT,
    POSTFLIGHT,
    EVT_BTN_LONG_HOLD,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Flight_To_Postflight"
  },

  // 8. DEVELOPMENT -> POSTFLIGHT (3-second user button long hold in DEVELOPMENT state triggers safe shutdown / SD flush)
  {
    DEVELOPMENT,
    POSTFLIGHT,
    EVT_BTN_LONG_HOLD,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Development_To_Postflight"
  }
};

static constexpr size_t NUM_TRANSITION_RULES = sizeof(TRANSITION_RULES) / sizeof(TRANSITION_RULES[0]);

// =============================================================================
// TRANSITION ENGINE HELPER FUNCTIONS
// =============================================================================
static bool check_match_policy(uint32_t current, uint32_t mask, MatchPolicy policy){
  if(policy == MatchPolicy::ALWAYS) return true;
  if(policy == MatchPolicy::NEVER) return false;
  if(policy == MatchPolicy::ALL) return (current & mask) == mask;
  if(policy == MatchPolicy::ANY) return (current & mask) != 0;
  return false;
}

static void evaluate_state_transitions(){
  if(!s_events_dirty) return; // Return immediately if no new events occur

  for(size_t i = 0; i < NUM_TRANSITION_RULES; ++i){
    const TransitRule& rule = TRANSITION_RULES[i];
    if(rule.from_state != s_current_state) continue;

    // Check blockers
    if(rule.blocker_mask != 0){
      bool blocked = check_match_policy(s_current_events, rule.blocker_mask, rule.block_policy);
      if(blocked) continue;
    }

    // Check triggers
    bool triggered = false;
    if(rule.trigger_policy == MatchPolicy::ALWAYS)
      triggered = true;
    else if(rule.trigger_mask != 0)
      triggered = check_match_policy(s_current_events, rule.trigger_mask, rule.trigger_policy);

    // Check negative triggers (must NOT be set)
    if(triggered && rule.neg_trigger_mask != 0 && (s_current_events & rule.neg_trigger_mask) != 0)
      triggered = false;

    if(triggered){
      Serial.printf("[FSM] Transition: %s (%d -> %d)\n", rule.rule_name, static_cast<int>(s_current_state), static_cast<int>(rule.to_state));
      SYS_ENTER_CRITICAL();
      s_current_state = rule.to_state;
      s_events_dirty = true; // New state requires re-evaluation of rules in next cycle
      SYS_EXIT_CRITICAL();
      return;
    }
  }

  s_events_dirty = false;
}


// =============================================================================
// MODULAR PRE-FLIGHT CHECK FUNCTIONS
// =============================================================================

// Check 2: Evaluates motion variance from raw IMU shared resources in ipc.h over 1.5s window
bool check_stationary_condition(){
  if((s_current_events & EVT_STATIONARY_FAIL) != 0) return false;
  if((s_current_events & EVT_STATIONARY_MET) != 0) return true;

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
      fsm_set_event(EVT_STATIONARY_MET);
      fsm_clear_event(EVT_STATIONARY_FAIL);
      Serial.println("[FSM] [GATE 2/6] Stationary condition check PASSED!");
      return true;
    }
  }else{
    stationary_start_ms = 0;
    fsm_clear_event(EVT_STATIONARY_MET);

    if((current_time_ms - check_start_ms) >= PREFLIGHT_STATIONARY_TIMEOUT_MS){
      fsm_set_event(EVT_STATIONARY_FAIL);
      Serial.printf("[FSM] [GATE 2/6] Stationary condition check FAILED! (Motion detected or settling timed out after %u ms).\n",
                    PREFLIGHT_STATIONARY_TIMEOUT_MS);
      return false;
    }
  }
  return false;
}

// Check 3: Evaluates resting bias from filtered sensor streams in ipc.h (awaiting calibration)
bool check_passive_sensor_calibration(){
  // If passive calibration already failed, return false immediately
  if((s_current_events & EVT_PASSIVE_CALIB_FAIL) != 0) return false;

  // If already evaluated and passed, return true immediately
  if((s_current_events & EVT_PASSIVE_CALIB_PASS) != 0) return true;

  // Only evaluate when drone is stationary
  if((s_current_events & EVT_STATIONARY_MET) == 0) return false;

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
    fsm_set_event(EVT_PASSIVE_CALIB_PASS);
    fsm_clear_event(EVT_PASSIVE_CALIB_FAIL);
    Serial.println("[FSM] [GATE 3/6] Passive calibration check PASSED!");
    return true;
  }else{
    fsm_set_event(EVT_PASSIVE_CALIB_FAIL);
    Serial.printf("[FSM] [GATE 3/6] Passive calibration check FAILED! (gyro_ok:%d, accel_dev:%.2fm/s^2, mag_norm:%.2f)\n",
                  gyro_bias_ok, accel_dev_mss, mag_norm);
    return false;
  }
}

// Check 4: Platform Tilt Check (Incline <= 10 deg)
bool check_platform_tilt(){
  if((s_current_events & EVT_TILT_CHECK_FAIL) != 0) return false;
  if((s_current_events & EVT_TILT_CHECK_PASS) != 0) return true;

  if(accel.empty()) return false;
  Vec3<float> a = accel.front();
  float a_norm = a.norm();
  if(!std::isfinite(a_norm) || a_norm < 0.5f || a_norm > 2.0f){
    fsm_set_event(EVT_TILT_CHECK_FAIL);
    fsm_clear_event(EVT_TILT_CHECK_PASS);
    Serial.printf("[FSM] [GATE 4/6] Platform tilt check FAILED! Invalid accel norm: %.2f g\n", a_norm);
    return false;
  }

  // Calculate tilt angle theta relative to gravity Z-axis: cos(theta) = az / |a|
  float cos_tilt = a.z / a_norm;
  if(cos_tilt > 1.0f) cos_tilt = 1.0f;
  if(cos_tilt < -1.0f) cos_tilt = -1.0f;
  float tilt_deg = acosf(cos_tilt) * (180.0f / 3.14159265f);

  if(tilt_deg <= PREFLIGHT_MAX_TILT_DEG){
    fsm_set_event(EVT_TILT_CHECK_PASS);
    fsm_clear_event(EVT_TILT_CHECK_FAIL);
    Serial.printf("[FSM] [GATE 4/6] Platform tilt check PASSED! (Incline: %.1f deg <= %.1f deg)\n",
                  tilt_deg, PREFLIGHT_MAX_TILT_DEG);
    return true;
  }else{
    fsm_set_event(EVT_TILT_CHECK_FAIL);
    fsm_clear_event(EVT_TILT_CHECK_PASS);
    Serial.printf("[FSM] [GATE 4/6] Platform tilt check FAILED! (Incline: %.1f deg > %.1f deg)\n",
                  tilt_deg, PREFLIGHT_MAX_TILT_DEG);
    return false;
  }
}

// Check 1: Sensor Data Stream & Hardware Bus Health Check
bool sensor_data_stream_check(){
  // If sensor stream healthcheck already permanently failed, return false immediately
  if((s_current_events & EVT_SENSOR_STREAM_FAIL) != 0) return false;

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
      if((s_current_events & EVT_SENSOR_STREAM_OK) == 0){
        fsm_set_event(EVT_SENSOR_STREAM_OK);
        fsm_clear_event(EVT_SENSOR_STREAM_FAIL);
        Serial.println("[FSM] [GATE 1/6] Sensor data stream check PASSED! All streams active & healthy.");
      }
      return true;
    }
  }else{
    healthy_start_ms = 0;

    // If stream was previously OK and suddenly dropped:
    if((s_current_events & EVT_SENSOR_STREAM_OK) != 0){
      fsm_clear_event(EVT_SENSOR_STREAM_OK);
      fsm_set_event(EVT_SENSOR_STREAM_FAIL);
      Serial.println("[FSM] [GATE 1/6] Sensor data stream DROPPED while in PREFLIGHT!");
      return false;
    }

    // Startup timeout check: If sensors failed to produce healthy streams within grace period
    if((now_ms - stream_check_start_ms) >= SENSOR_STREAM_STARTUP_TIMEOUT_MS){
      fsm_set_event(EVT_SENSOR_STREAM_FAIL);
      Serial.println("[FSM] [GATE 1/6] Sensor data stream check TIMED OUT / FAILED!");
      if(!has_data){
        Serial.printf("      Missing data: IMU_raw:%d, MAG_raw:%d, BARO_raw:%d, IMU_filt:%d, MAG_filt:%d\n",
                      imu_raw_has_data, mag_raw_has_data, baro_raw_has_data, imu_filt_has_data, mag_filt_has_data);
      }
      if(!fresh){
        Serial.printf("      Stale streams: IMU_raw:%d, IMU_filt:%d, MAG_raw:%d, MAG_filt:%d, BARO_raw:%d\n",
                      imu_raw_fresh, imu_filt_fresh, mag_raw_fresh, mag_filt_fresh, baro_raw_fresh);
      }
      if(!valid){
        Serial.printf("      Invalid ranges: IMU:%d, MAG:%d, BARO:%d (a:%.2f, m:%.2f, p:%.1f)\n",
                      imu_valid, mag_valid, baro_valid, a_sample.norm(), m_sample.norm(), p_sample);
      }
      return false;
    }
  }

  return ((s_current_events & EVT_SENSOR_STREAM_OK) != 0);
}

// Check 5: Power Module Visibility & Initial Voltage Health Check
bool check_power_initial_condition(){
  if((s_current_events & EVT_BATTERY_LOW) != 0) return false;
  if((s_current_events & EVT_BATTERY_VOLTAGE_OK) != 0) return true;

  static uint32_t power_check_start_ms = 0;
  uint32_t now_ms = millis();
  if(power_check_start_ms == 0) power_check_start_ms = now_ms;

  if(power_data.empty()){
    if((now_ms - power_check_start_ms) >= SENSOR_STREAM_STARTUP_TIMEOUT_MS){
      fsm_set_event(EVT_BATTERY_LOW);
      fsm_clear_event(EVT_BATTERY_VOLTAGE_OK);
      Serial.println("[FSM] [GATE 5/6] Battery check TIMED OUT / FAILED! (No power module or battery detected). Transitioning to DEVELOPMENT.");
      return false;
    }
    return false;
  }

  PowerMeasurement p = power_data.front();
  float v_mv = p.voltage_mv;

  // Numerical validity check
  if(!std::isfinite(v_mv) || v_mv < 1000.0f || v_mv > 36000.0f ||
     !std::isfinite(p.current_ma) || !std::isfinite(p.power_mw)){
    fsm_set_event(EVT_BATTERY_LOW);
    fsm_clear_event(EVT_BATTERY_VOLTAGE_OK);
    Serial.printf("[FSM] [GATE 5/6] Power module initial check FAILED! Invalid reading: %.1f mV. Transitioning to DEVELOPMENT.\n", v_mv);
    return false;
  }

  // Auto-detect cell count & calculate per-cell voltage
  int32_t cell_count = static_cast<int32_t>(roundf(v_mv / 3800.0f));
  if(cell_count < 1) cell_count = 1;
  if(cell_count > 8) cell_count = 8;
  float cell_v_mv = v_mv / static_cast<float>(cell_count);

  if(cell_v_mv < BATTERY_CELL_MIN_VOLTAGE_MV){
    fsm_set_event(EVT_BATTERY_LOW);
    fsm_clear_event(EVT_BATTERY_VOLTAGE_OK);
    Serial.printf("[FSM] [GATE 5/6] Battery initial check FAILED! Pack: %.1f mV (%.1f mV/cell on %dS < %.1f mV/cell). Transitioning to DEVELOPMENT.\n",
                  v_mv, cell_v_mv, static_cast<int>(cell_count), BATTERY_CELL_MIN_VOLTAGE_MV);
    return false;
  }

  fsm_set_event(EVT_BATTERY_VOLTAGE_OK);
  fsm_clear_event(EVT_BATTERY_LOW);
  Serial.printf("[FSM] [GATE 5/6] Power module & battery initial check PASSED! Pack: %.1f mV (%.1f mV/cell on %dS)\n",
                v_mv, cell_v_mv, static_cast<int>(cell_count));
  return true;
}

// Check 6: RC Controller Link & Failsafe Check
bool check_rc_controller_signal(){
  if((s_current_events & EVT_RC_SIGNAL_FAIL) != 0) return false;
  if((s_current_events & EVT_RC_SIGNAL_OK) != 0) return true;

  static uint32_t rc_check_start_ms = 0;
  uint32_t now_ms = millis();
  if(rc_check_start_ms == 0) rc_check_start_ms = now_ms;

  if(rc_data.empty()){
    if((now_ms - rc_check_start_ms) >= SENSOR_STREAM_STARTUP_TIMEOUT_MS){
      fsm_set_event(EVT_RC_SIGNAL_FAIL);
      fsm_clear_event(EVT_RC_SIGNAL_OK);
      Serial.println("[FSM] [GATE 6/6] RC receiver signal check TIMED OUT / FAILED! (No RC stream detected). Transitioning to DEVELOPMENT.");
      return false;
    }
    return false;
  }

  const RcChannels& ch = rc_data.front();
  if(ch.failsafe){
    fsm_set_event(EVT_RC_SIGNAL_FAIL);
    fsm_clear_event(EVT_RC_SIGNAL_OK);
    Serial.println("[FSM] [GATE 6/6] RC receiver signal check FAILED! (Failsafe active). Transitioning to DEVELOPMENT.");
    return false;
  }

  fsm_set_event(EVT_RC_SIGNAL_OK);
  fsm_clear_event(EVT_RC_SIGNAL_FAIL);
  Serial.printf("[FSM] [GATE 6/6] RC receiver signal check PASSED! (Signal locked, active channels: %u)\n",
                ch.channel_count);
  return true;
}

// Check 7 (Future): Storage Check (SD card mount status and capacity)
static bool check_storage_health(){
  // [FUTURE CHECK] Verify SD card sector capacity and mount status
  return true;
}

// =============================================================================
// MODULAR STATE HANDLER FUNCTIONS
// =============================================================================
static void handle_state_uninit(){
  evaluate_state_transitions();
}

static void handle_state_calib(uint32_t loop_count){
  static CalibSubState s_sub_state = CALIB_SUBSTATE_SELECT_MENU;
  static uint8_t s_sensor_menu_idx = 0;
  static bool s_menu_initialized = false;

  const uint32_t dwell_loops = (CALIB_MENU_DWELL_MS * TASK_FSM_FREQ_HZ) / 1000;

  switch(s_sub_state){
    case CALIB_SUBSTATE_SELECT_MENU: {
      if(!s_menu_initialized || (loop_count % dwell_loops == 0)){
        if(s_menu_initialized) s_sensor_menu_idx = (s_sensor_menu_idx + 1) % 4;
        s_menu_initialized = true;

        LedCommand cmd = {LED_PATTERN_SOLID, get_sensor_menu_color(s_sensor_menu_idx)};
        xQueueSend(led_cmd_queue, &cmd, 0);

        const char* sensor_name = (s_sensor_menu_idx == 0) ? "GYROSCOPE (Cyan)" :
                                  (s_sensor_menu_idx == 1) ? "ACCELEROMETER (Orange)" :
                                  (s_sensor_menu_idx == 2) ? "MAGNETOMETER (Magenta)" : "BAROMETER (Blue)";
        Serial.printf("[FSM_CALIB] Sensor Menu: %s. Click button to calibrate.\n", sensor_name);
      }

      // Check button short-click to lock in sensor
      if((s_current_events & EVT_BTN_SHORT_CLICK) != 0){
        fsm_clear_event(EVT_BTN_SHORT_CLICK);
        Serial.printf("[FSM_CALIB] Selected sensor index %d! Spawning calibration task...\n", s_sensor_menu_idx);

        // Visual ACK: White flash burst
        LedCommand ack_cmd = {LED_PATTERN_FLASH_BURST, PALETTE_WHITE};
        xQueueSend(led_cmd_queue, &ack_cmd, 0);

        // Spawn specific calibration task
        switch(s_sensor_menu_idx){
          case 0:
            xTaskCreatePinnedToCore(task_calib_gyro, TASK_CALIB_GYRO_NAME, TASK_CALIB_GYRO_STACK_SIZE,
                                    nullptr, TASK_CALIB_GYRO_PRIORITY, &s_active_calib_handle, TASK_CALIB_GYRO_CORE);
            break;
          case 1:
            xTaskCreatePinnedToCore(task_calib_accel, TASK_CALIB_ACCEL_NAME, TASK_CALIB_ACCEL_STACK_SIZE,
                                    nullptr, TASK_CALIB_ACCEL_PRIORITY, &s_active_calib_handle, TASK_CALIB_ACCEL_CORE);
            break;
          case 2:
            xTaskCreatePinnedToCore(task_calib_mag, TASK_CALIB_MAG_NAME, TASK_CALIB_MAG_STACK_SIZE,
                                    nullptr, TASK_CALIB_MAG_PRIORITY, &s_active_calib_handle, TASK_CALIB_MAG_CORE);
            break;
          case 3:
            xTaskCreatePinnedToCore(task_calib_baro, TASK_CALIB_BARO_NAME, TASK_CALIB_BARO_STACK_SIZE,
                                    nullptr, TASK_CALIB_BARO_PRIORITY, &s_active_calib_handle, TASK_CALIB_BARO_CORE);
            break;
        }

        s_sub_state = CALIB_SUBSTATE_MONITORING;
      }
      break;
    }

    case CALIB_SUBSTATE_MONITORING: {
      // Monitor messages and reports from the spawned calib task
      CalibReportMsg rpt;
      while(xQueueReceive(calib_report_queue, &rpt, 0) == pdTRUE){
        if(rpt.status == CALIB_STATUS_SETTLING){
          Serial.printf("[FSM_CALIB] Sensor %d entered settling phase. Setting fast Amber warning LED...\n", rpt.target);
          LedCommand settle_cmd = {LED_PATTERN_BLINK_FAST, PALETTE_AMBER};
          xQueueSend(led_cmd_queue, &settle_cmd, 0);
        }else if(rpt.status == CALIB_STATUS_SAMPLING){
          Serial.printf("[FSM_CALIB] Sensor %d started sampling. Setting breathing LED...\n", rpt.target);
          LedCommand breathe_cmd = {LED_PATTERN_BREATHE, get_sensor_menu_color(s_sensor_menu_idx)};
          xQueueSend(led_cmd_queue, &breathe_cmd, 0);
        }else if(rpt.status == CALIB_STATUS_STEP_LOCKED){
          Serial.printf("[FSM_CALIB] Step %d locked! Triggering visual cue.\n", rpt.current_step);
          LedCommand flash_cue = {LED_PATTERN_FLASH_BURST, PALETTE_WHITE};
          xQueueSend(led_cmd_queue, &flash_cue, 0);
        }else if(rpt.status == CALIB_STATUS_HEALTHCHECK){
          if(loop_count % 100 == 0)
            Serial.printf("[FSM_CALIB] Healthcheck: sensor %d, step %d, progress %d%%\n",
                          rpt.target, rpt.current_step, rpt.progress_pct);
        }else if(rpt.status == CALIB_STATUS_COMPUTING){
          Serial.printf("[FSM_CALIB] Sensor %d computing optimization. Setting fast blink LED...\n", rpt.target);
          LedCommand comp_cmd = {LED_PATTERN_BLINK_FAST, get_sensor_menu_color(s_sensor_menu_idx)};
          xQueueSend(led_cmd_queue, &comp_cmd, 0);
        }else if(rpt.status == CALIB_STATUS_COMPLETED){
          Serial.println("[FSM_CALIB] Task reported completion! Destroying worker task...");
          if(s_active_calib_handle != nullptr){
            vTaskDelete(s_active_calib_handle);
            s_active_calib_handle = nullptr;
          }
          LedCommand green_cmd = {LED_PATTERN_SOLID, PALETTE_GREEN};
          xQueueSend(led_cmd_queue, &green_cmd, 0);

          fsm_clear_event(EVT_ACTIVE_CALIB_INVOKED);
          fsm_set_event(EVT_CALIB_COMPLETE);
          s_sub_state = CALIB_SUBSTATE_DONE;
        }else if(rpt.status == CALIB_STATUS_FAILED){
          Serial.println("[FSM_CALIB] Task reported failure! Destroying worker task...");
          if(s_active_calib_handle != nullptr){
            vTaskDelete(s_active_calib_handle);
            s_active_calib_handle = nullptr;
          }
          LedCommand red_cmd = {LED_PATTERN_BLINK_FAST, PALETTE_RED};
          xQueueSend(led_cmd_queue, &red_cmd, 0);

          fsm_clear_event(EVT_ACTIVE_CALIB_INVOKED);
          fsm_set_event(EVT_CALIB_FAILED);
          s_sub_state = CALIB_SUBSTATE_FAILED;
        }
      }

      // Interactive user button click (e.g. Stop acquisition for continuous mag tumble)
      if((s_current_events & EVT_BTN_SHORT_CLICK) != 0){
        fsm_clear_event(EVT_BTN_SHORT_CLICK);
        if(s_active_calib_handle != nullptr){
          xTaskNotifyGive(s_active_calib_handle);
          Serial.println("[FSM_CALIB] User button clicked: signaled active calibration task!");
          LedCommand ack_cmd = {LED_PATTERN_FLASH_BURST, PALETTE_WHITE};
          xQueueSend(led_cmd_queue, &ack_cmd, 0);
        }
      }

      // User cancellation check (Long press on button aborts back to menu)
      if((s_current_events & EVT_BTN_LONG_HOLD) != 0){
        fsm_clear_event(EVT_BTN_LONG_HOLD);
        Serial.println("[FSM_CALIB] User aborted calibration via long press! Destroying worker task...");
        if(s_active_calib_handle != nullptr){
          vTaskDelete(s_active_calib_handle);
          s_active_calib_handle = nullptr;
        }
        LedCommand red_cue = {LED_PATTERN_BLINK_FAST, PALETTE_RED};
        xQueueSend(led_cmd_queue, &red_cue, 0);

        s_menu_initialized = false;
        s_sub_state = CALIB_SUBSTATE_SELECT_MENU;
      }
      break;
    }

    case CALIB_SUBSTATE_DONE: {
      if constexpr(AUTO_REBOOT_AFTER_CALIBRATION){
        Serial.println("[FSM_CALIB] AUTO_REBOOT enabled. Rebooting in 2.0 seconds...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
      }
      break;
    }

    case CALIB_SUBSTATE_FAILED: {
      break;
    }
  }
}

static void handle_state_preflight(){
  // [SAFETY MOTOR LOCKOUT] Ensure all motor PWM outputs are disabled (PWM = 0)
  // hal_motor_disable_all(); // Motor driver PWM = 0 lockout

  // Check if user requested active sensor calibration (held at boot or pressed in preflight)
  if((s_current_events & (EVT_BTN_LONG_HOLD | EVT_ACTIVE_CALIB_INVOKED)) != 0 || user_button_is_pressed()){
    Serial.println("[FSM] User button pressed in PREFLIGHT: Triggering active calibration mode...");
    fsm_set_event(EVT_ACTIVE_CALIB_INVOKED);
    evaluate_state_transitions();
    return;
  }

  // Evaluate 6 mandatory pre-flight safety gates
  sensor_data_stream_check();
  check_stationary_condition();
  check_passive_sensor_calibration();
  check_platform_tilt();
  check_power_initial_condition();
  check_rc_controller_signal();

  evaluate_state_transitions();
}

static void handle_state_development(){
  // [SAFETY MOTOR LOCKOUT] Ensure all motor PWM outputs remain disabled (PWM = 0)
  // hal_motor_disable_all(); // Motor driver PWM = 0 lockout

  static bool s_dev_entry_logged = false;
  if(!s_dev_entry_logged){
    s_dev_entry_logged = true;
    Serial.println("[FSM] ENTERED DEVELOPMENT STATE! Pre-flight check failed. Diagnostic & inspection mode active.");
    LedCommand dev_cmd = {LED_PATTERN_BLINK_SLOW, PALETTE_ORANGE};
    xQueueSend(led_cmd_queue, &dev_cmd, 0);
    // Clear any prior stale button clicks on initial entry into DEVELOPMENT
    fsm_clear_event(EVT_BTN_SHORT_CLICK | EVT_BTN_LONG_HOLD);
  }

  evaluate_state_transitions();
}

static void handle_state_ready(){
  evaluate_state_transitions();
}

static void handle_state_flight(){
  evaluate_state_transitions();
}

static void handle_state_postflight(){
  static bool s_postflight_actions_done = false;
  if(!s_postflight_actions_done){
    s_postflight_actions_done = true;
    Serial.println("[FSM] Entered POSTFLIGHT! Shutting down SD card logger safely...");
    sd_stop_logging_requested = true;
    if(app_task_handles[TASK_SD_LOGGER_ID] != nullptr)
      xTaskNotifyGive(app_task_handles[TASK_SD_LOGGER_ID]);
    LedCommand post_cmd = {LED_PATTERN_FLASH_BURST, PALETTE_CYAN};
    xQueueSend(led_cmd_queue, &post_cmd, 0);
  }
  evaluate_state_transitions();
}


static void handle_state_emergency(){
  evaluate_state_transitions();
}

// =============================================================================
// FSM PUBLIC API IMPLEMENTATIONS
// =============================================================================
void fsm_init(){
  s_current_state = UNINIT;
  s_current_events = EVT_NONE;
  s_events_dirty = true;
}

SystemState fsm_get_state(){
  return s_current_state;
}

uint32_t fsm_get_events(){
  return s_current_events;
}

void fsm_set_event(uint32_t event_mask){
  SYS_ENTER_CRITICAL();
  s_current_events |= event_mask;
  s_events_dirty = true;
  SYS_EXIT_CRITICAL();
}

void fsm_clear_event(uint32_t event_mask){
  SYS_ENTER_CRITICAL();
  s_current_events &= ~event_mask;
  s_events_dirty = true;
  SYS_EXIT_CRITICAL();
}

void fsm_trigger_emergency(){
  fsm_set_event(EVT_EMERGENCY_SHUTDOWN);
  if(app_task_handles[TASK_FSM_ID] != nullptr)
    xTaskNotifyGive(app_task_handles[TASK_FSM_ID]);
}

bool fsm_is_armed_allowed(){
  return (s_current_state == READY || s_current_state == FLIGHT || s_current_state == TAKEOFF);
}

// =============================================================================
// FSM CORE 100 HZ TASK RUNNER
// =============================================================================
void fsm_task(void* arg){
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_FSM_FREQ_HZ);
  TickType_t last_wake_time = xTaskGetTickCount();
  uint32_t loop_count = 0;

  while(true){
    // Non-blocking drain of any emergency direct task notification
    if(ulTaskNotifyTake(pdTRUE, 0) > 0)
      Serial.println("[FSM] Task notification received!");

    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2))
      last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(TASK_FSM_ID); //======================================================

    uint8_t btn_evts = ipc_pop_btn_events();
    if((btn_evts & BTN_EVT_SHORT_CLICK) != 0) fsm_set_event(EVT_BTN_SHORT_CLICK);
    if((btn_evts & BTN_EVT_LONG_HOLD) != 0) fsm_set_event(EVT_BTN_LONG_HOLD);
    
    loop_count++;

    switch(s_current_state){
      case UNINIT:      handle_state_uninit();                  break;
      case CALIB:       handle_state_calib(loop_count);         break;
      case PREFLIGHT:   handle_state_preflight();               break;
      case READY:       handle_state_ready();                   break;
      case TAKEOFF:                                             break;
      case FLIGHT:      handle_state_flight();                  break;
      case DEVELOPMENT: handle_state_development();             break;
      case LANDING:                                             break; //[NOTE] Temporary Ignored/bypassed: 
      case POSTFLIGHT:  handle_state_postflight();              break;
      case LEVELOUT:                                            break;
      case CRASHED:                                             break;
      case FREEFALL:    handle_state_emergency();               break;
      default:                                                  break;
    }

    SYSVIEW_END(TASK_FSM_ID); //==================================================
  }
}
