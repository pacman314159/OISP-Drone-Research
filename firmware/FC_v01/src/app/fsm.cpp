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
  // 1. UNINIT -> CALIB (If user held button at boot)
  {
    UNINIT,
    CALIB,
    EVT_ACTIVE_CALIB_INVOKED,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Uninit_To_Calib"
  },

  // 2. UNINIT -> PREFLIGHT (Normal boot sequence when no calibration invoked)
  {
    UNINIT,
    PREFLIGHT,
    EVT_NONE,
    EVT_ACTIVE_CALIB_INVOKED,
    MatchPolicy::ALWAYS,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Uninit_To_Preflight"
  },

  // 3. CALIB -> PREFLIGHT (Calibration finished successfully)
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

  // 4. PREFLIGHT -> READY (Stationary + Passive calib passed; no calib fail blocker)
  {
    PREFLIGHT,
    READY,
    EVT_STATIONARY_MET | EVT_PASSIVE_CALIB_PASS,
    EVT_NONE,
    MatchPolicy::ALL,
    EVT_EMERGENCY_SHUTDOWN | EVT_PASSIVE_CALIB_FAIL,
    MatchPolicy::ANY,
    "Rule_Preflight_To_Ready"
  },

  // 5. READY -> FLIGHT (Arming / Takeoff command - future expansion)
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

  // 6. FLIGHT -> POSTFLIGHT (Temporary bypass around LANDING: 3s button hold terminates flight)
  {
    FLIGHT,
    POSTFLIGHT,
    EVT_BTN_LONG_HOLD,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Flight_To_Postflight"
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

// Visual error latch: blink RGB LED to indicate passive calibration out-of-tolerance
static void indicate_passive_calib_error(uint32_t loop_count){
  static bool s_alert_latched = false;
  if(!s_alert_latched){
    s_alert_latched = true;
    LedCommand cmd = {LED_PATTERN_BLINK_SLOW, PALETTE_AMBER};
    xQueueSend(led_cmd_queue, &cmd, 0);
  }
}

// =============================================================================
// MODULAR PRE-FLIGHT CHECK FUNCTIONS
// =============================================================================

// Check 1: Evaluates motion variance from raw IMU shared resources in ipc.h over 1.5s window
static bool check_stationary_condition(){
  static Vec3<float> last_raw_accel{0.0f, 0.0f, 0.0f};
  static Vec3<float> last_raw_gyro{0.0f, 0.0f, 0.0f};
  static uint32_t stationary_start_ms = 0;

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

  uint32_t current_time_ms = millis();
  if(gyro_motion_low && accel_motion_low){
    if(stationary_start_ms == 0)
      stationary_start_ms = current_time_ms;
    else if((current_time_ms - stationary_start_ms) >= STATIONARY_TIME_REQUIRED_MS){
      if((s_current_events & EVT_STATIONARY_MET) == 0){
        fsm_set_event(EVT_STATIONARY_MET);
        Serial.println("[FSM] Stationary condition met!");
      }
      return true;
    }
  }else{
    stationary_start_ms = 0;
    fsm_clear_event(EVT_STATIONARY_MET);
  }
  return false;
}

// Check 2: Evaluates resting bias from filtered sensor streams in ipc.h (awaiting calibration)
static bool check_passive_sensor_calibration(){
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
    Serial.println("[FSM] Passive calibration check PASSED!");
    return true;
  }else{
    fsm_set_event(EVT_PASSIVE_CALIB_FAIL);
    Serial.printf("[FSM] Passive calibration FAILED! (gyro_ok:%d, accel_dev:%.2fm/s^2, mag_norm:%.2f)\n",
                  gyro_bias_ok, accel_dev_mss, mag_norm);
    return false;
  }
}

// Check 3 (Future): Platform Tilt Check (< 10 deg incline)
static bool check_platform_tilt(){
  // [FUTURE CHECK] Calculate roll/pitch from accel and set EVT_TILT_CHECK_PASS/FAIL
  return true;
}

// Check 4 (Future): Sensor Stream Check (I2C/SPI transaction rate and errors)
static bool check_sensor_stream_health(){
  // [FUTURE CHECK] Check bus error counters and timestamps for fresh data
  return true;
}

// Check 5 (Future): RC Controller Link Check
static bool check_rc_controller_signal(){
  // [FUTURE CHECK] Validate receiver frames and failsafe flags
  return true;
}

// Check 6 (Future): Storage Check (SD card mount status and capacity)
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

static void handle_state_preflight(uint32_t loop_count){
  // If passive calibration already failed, permanently latch visual alert pattern
  if((s_current_events & EVT_PASSIVE_CALIB_FAIL) != 0){
    indicate_passive_calib_error(loop_count);
    return;
  }

  // Run modular preflight check pipeline reading from ipc.h shared resources.
  // Sequential early-exit: If any check fails, immediately skip remaining checks.
  if(!check_stationary_condition()) return;
  if(!check_passive_sensor_calibration()) return;
  if(!check_platform_tilt()) return;
  if(!check_sensor_stream_health()) return;
  if(!check_rc_controller_signal()) return;
  if(!check_storage_health()) return;

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

  // Step 1: Sample USER_BTN directly on startup to detect active calibration invoke
  vTaskDelay(pdMS_TO_TICKS(50)); // Settle GPIO debounce
  if(user_button_is_pressed()){
    Serial.println("[FSM] USER_BTN held at boot! Triggering EVT_ACTIVE_CALIB_INVOKED");
    fsm_set_event(EVT_ACTIVE_CALIB_INVOKED);
  }

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
      case PREFLIGHT:   handle_state_preflight(loop_count);     break;
      case READY:       handle_state_ready();                   break;
      case TAKEOFF:                                             break;
      case FLIGHT:      handle_state_flight();                  break;
      //[NOTE] Temporary Ignored/bypassed: 
      case LANDING:                                             break;
      case POSTFLIGHT:  handle_state_postflight();              break;
      case LEVELOUT:                                            break;
      case CRASHED:                                             break;
      case FREEFALL:    handle_state_emergency();               break;
      default:                                                  break;
    }

    SYSVIEW_END(TASK_FSM_ID); //==================================================
  }
}
