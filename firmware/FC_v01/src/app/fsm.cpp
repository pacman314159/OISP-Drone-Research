#include "app/fsm.h"
#include <Arduino.h>
#include <cmath>
#include <cstdlib>
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/critical_section.h"
#include "middleware/sysview_tracing.h"
#include "drivers/led/ws2812.h"
#include "core/interface/user_button.h"
#include "core/interface/interface_tasks.h"
#include "core/sensor/calib_tasks.h"
#include "core/sensor/calib.h"
#include "core/safety/preflight_checks.h"
#include "core/blackbox/sd_logger_task.h"

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
static bool s_ready_baro_calibrated = false;

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

  // Architectural Design Decision (Zettel 0040):
  // No transition rule exists from CALIB back to PREFLIGHT or normal flight states.
  // Calibration permanently modifies persistent sensor bias/scale calibration in Flash NVS.
  // To guarantee deterministic state, flush sensor preprocessing pipelines, and prevent
  // stale calibration data leakage into flight loops, exiting CALIB requires a clean system
  // reboot (either via AUTO_REBOOT_AFTER_CALIBRATION or a manual power-cycle/reset).

  // 3. PREFLIGHT -> READY (All 6 Mandatory Safety Gates Passed; No Fail Blockers)
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

  // 6. READY -> TAKEOFF (Pilot commanded arm / takeoff initiation)
  {
    READY,
    TAKEOFF,
    EVT_TAKEOFF_INITIATED,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN | EVT_RC_SIGNAL_FAIL | EVT_BATTERY_LOW,
    MatchPolicy::ANY,
    "Rule_Ready_To_Takeoff"
  },

  // 7. TAKEOFF -> FLIGHT (5-second takeoff window satisfied with all safety guards intact)
  {
    TAKEOFF,
    FLIGHT,
    EVT_TAKEOFF_COMPLETE,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN | EVT_RC_SIGNAL_FAIL | EVT_TAKEOFF_SAFETY_VIOLATION,
    MatchPolicy::ANY,
    "Rule_Takeoff_To_Flight"
  },

  // 8. TAKEOFF -> READY (Pilot aborted takeoff / throttled down to idle during ground pacing)
  {
    TAKEOFF,
    READY,
    EVT_TAKEOFF_ABORT,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_EMERGENCY_SHUTDOWN,
    MatchPolicy::ANY,
    "Rule_Takeoff_To_Ready"
  },

  // 9. TAKEOFF -> CRASHED (Safety violation or emergency during takeoff)
  {
    TAKEOFF,
    CRASHED,
    EVT_TAKEOFF_SAFETY_VIOLATION | EVT_EMERGENCY_SHUTDOWN,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_NONE,
    MatchPolicy::NEVER,
    "Rule_Takeoff_To_Crashed"
  },

  // 10. FLIGHT -> POSTFLIGHT (Temporary bypass around LANDING: 3s button hold terminates flight)
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

  // 11. FLIGHT -> CRASHED (Emergency shutdown during flight)
  {
    FLIGHT,
    CRASHED,
    EVT_EMERGENCY_SHUTDOWN,
    EVT_NONE,
    MatchPolicy::ANY,
    EVT_NONE,
    MatchPolicy::NEVER,
    "Rule_Flight_To_Crashed"
  },

  // 12. DEVELOPMENT -> POSTFLIGHT (3-second user button long hold in DEVELOPMENT state triggers safe shutdown / SD flush)
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

static void fsm_on_state_transition(SystemState from, SystemState to){
  LedCommand cmd{LED_PATTERN_OFF, PALETTE_OFF};
  bool dispatch_led = true;

  switch(to){
    case UNINIT:
      cmd = {LED_PATTERN_OFF, PALETTE_OFF};
      break;

    case PREFLIGHT:
      cmd = {LED_PATTERN_SOLID, PALETTE_CYAN};
      break;

    case CALIB:
      // Active calibration coordinator directly commands interactive sensor-specific LEDs
      dispatch_led = false;
      break;

    case READY:
      cmd = {LED_PATTERN_SOLID, PALETTE_GREEN};
      s_ready_baro_calibrated = false;
      if(!pressure.empty()){
        float p_ground = pressure.front();
        if(p_ground >= SENSOR_STREAM_BARO_MIN_PA && p_ground <= SENSOR_STREAM_BARO_MAX_PA){
          calib_data.baro_ref_pa = p_ground;
          s_ready_baro_calibrated = true;
          Serial.printf("[FSM] READY Transition: Calibrated barometer ground reference P0 to %.2f Pa (Option A)\n", p_ground);
        }
      }
      break;

    case TAKEOFF:
      cmd = {LED_PATTERN_BLINK_FAST, PALETTE_LIME};
      break;

    case FLIGHT:
      cmd = {LED_PATTERN_SOLID, PALETTE_BLUE};
      break;

    case LANDING:
      cmd = {LED_PATTERN_SOLID, PALETTE_TEAL};
      break;

    case POSTFLIGHT:
      cmd = {LED_PATTERN_SOLID, PALETTE_PURPLE};
      break;

    case DEVELOPMENT:
      cmd = {LED_PATTERN_SOLID, PALETTE_YELLOW};
      break;

    case LEVELOUT:
      cmd = {LED_PATTERN_BLINK_FAST, PALETTE_ORANGE};
      break;

    case CRASHED:
      cmd = {LED_PATTERN_BLINK_FAST, PALETTE_RED};
      break;

    case FREEFALL:
      cmd = {LED_PATTERN_FLASH_BURST, PALETTE_RED};
      break;

    default:
      dispatch_led = false;
      break;
  }

  if(dispatch_led && led_cmd_queue != nullptr)
    xQueueSend(led_cmd_queue, &cmd, 0);
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
      SystemState prev_state = s_current_state;
      s_current_state = rule.to_state;
      s_events_dirty = true; // New state requires re-evaluation of rules in next cycle
      SYS_EXIT_CRITICAL();
      fsm_on_state_transition(prev_state, rule.to_state);
      return;
    }
  }

  s_events_dirty = false;
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
      // Architectural Design Decision (Zettel 0040): Terminal calibration state.
      // System intentionally remains in CALIB until rebooted to ensure a deterministic cold-boot
      // with newly written NVS calibration parameters and flush all estimator/filter buffers.
      if constexpr(AUTO_REBOOT_AFTER_CALIBRATION){
        Serial.println("[FSM_CALIB] AUTO_REBOOT enabled. Rebooting in 2.0 seconds...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_restart();
      }
      break;
    }

    case CALIB_SUBSTATE_FAILED: {
      // Architectural Design Decision (Zettel 0040): Terminal calibration failure state.
      // Drone holds in CALIB with visual error LED cue until manually reset or power-cycled.
      break;
    }
  }
}

static void update_gate_event(SafetyStatus status, uint32_t pass_evt, uint32_t fail_evt){
  if(status == SafetyStatus::PASSED){
    fsm_set_event(pass_evt);
    fsm_clear_event(fail_evt);
  }else if(status == SafetyStatus::FAILED){
    fsm_clear_event(pass_evt);
    fsm_set_event(fail_evt);
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
  SafetyStatus stream_status = check_sensor_data_stream();
  update_gate_event(stream_status, EVT_SENSOR_STREAM_OK, EVT_SENSOR_STREAM_FAIL);

  SafetyStatus stat_status = check_stationary_condition();
  update_gate_event(stat_status, EVT_STATIONARY_MET, EVT_STATIONARY_FAIL);

  bool is_stationary = (stat_status == SafetyStatus::PASSED);
  SafetyStatus calib_status = check_passive_sensor_calibration(is_stationary);
  update_gate_event(calib_status, EVT_PASSIVE_CALIB_PASS, EVT_PASSIVE_CALIB_FAIL);

  SafetyStatus tilt_status = check_platform_tilt();
  update_gate_event(tilt_status, EVT_TILT_CHECK_PASS, EVT_TILT_CHECK_FAIL);

  SafetyStatus power_status = check_power_initial_condition();
  update_gate_event(power_status, EVT_BATTERY_VOLTAGE_OK, EVT_BATTERY_LOW);

  SafetyStatus rc_status = check_rc_controller_signal();
  update_gate_event(rc_status, EVT_RC_SIGNAL_OK, EVT_RC_SIGNAL_FAIL);

  evaluate_state_transitions();
}

static void handle_state_development(){
  // [SAFETY MOTOR LOCKOUT] Ensure all motor PWM outputs remain disabled (PWM = 0)
  // hal_motor_disable_all(); // Motor driver PWM = 0 lockout

  static bool s_dev_entry_logged = false;
  if(!s_dev_entry_logged){
    s_dev_entry_logged = true;
    Serial.println("[FSM] ENTERED DEVELOPMENT STATE! Pre-flight check failed. Diagnostic & inspection mode active.");
    // Clear any prior stale button clicks on initial entry into DEVELOPMENT
    fsm_clear_event(EVT_BTN_SHORT_CLICK | EVT_BTN_LONG_HOLD);
  }

  evaluate_state_transitions();
}

static void handle_state_ready(){
  // Action 1: Calibrating the height measured from the barometer (Option A: ground pressure reference)
  if(!s_ready_baro_calibrated && !pressure.empty()){
    float p_ground = pressure.front();
    if(p_ground >= SENSOR_STREAM_BARO_MIN_PA && p_ground <= SENSOR_STREAM_BARO_MAX_PA){
      calib_data.baro_ref_pa = p_ground;
      s_ready_baro_calibrated = true;
      Serial.printf("[FSM] READY State: Calibrated barometer ground reference P0 to %.2f Pa (Option A)\n", p_ground);
    }
  }
  // Action 2: Set the start up heading angle (measured from ekf, magnetometer)
  // Action 3: Start the drone at idle throttle (timeout: 2s), do not lend pilot controls yet

  if(!rc_data.empty()){
    const RcChannels& ch = rc_data.front();
    if(ch.failsafe){
      evaluate_state_transitions();
      return;
    }

    // Arming trigger: Arm switch active (Ch5 > 1500 us) OR pilot raises throttle above idle
    bool arm_switch_active = (ch.channel_count >= 5) && (ch.channels[4] > RC_ARM_SWITCH_THRESHOLD_US);
    bool throttle_elevated = (ch.channels[2] > RC_THROTTLE_IDLE_US);

    // Require stick centering guard: roll, pitch, yaw within center deadband (+-50 us)
    bool roll_centered   = abs(static_cast<int>(ch.channels[0]) - 1500) <= RC_STICK_CENTER_DEADBAND_US;
    bool pitch_centered  = abs(static_cast<int>(ch.channels[1]) - 1500) <= RC_STICK_CENTER_DEADBAND_US;
    bool yaw_centered    = abs(static_cast<int>(ch.channels[3]) - 1500) <= RC_STICK_CENTER_DEADBAND_US;
    bool sticks_centered = roll_centered && pitch_centered && yaw_centered;

    if((arm_switch_active || throttle_elevated) && sticks_centered){
      Serial.println("[FSM] Pilot armed / commanded takeoff! Initiating TAKEOFF sequence...");
      fsm_set_event(EVT_TAKEOFF_INITIATED);
      fsm_clear_event(EVT_TAKEOFF_ABORT);

      if(motor_cmd_queue != nullptr){
        MotorCommand cmd = MOTOR_CMD_ARM;
        xQueueSend(motor_cmd_queue, &cmd, 0);
      }
    }
  }

  evaluate_state_transitions();
}

static void handle_state_takeoff(){
  static bool s_takeoff_initialized = false;
  static uint32_t s_takeoff_window_start_ms = 0;
  static bool s_pilot_control_latched = false;
  static float s_startup_heading_deg = 0.0f;

  if(!s_takeoff_initialized){
    s_takeoff_initialized = true;
    s_takeoff_window_start_ms = 0;
    s_pilot_control_latched = false;

    // Capture initial heading reference from EKF attitude
    if(!att_euler.empty()){
      Vec3<float> euler = att_euler.front();
      s_startup_heading_deg = euler.z;
    }else{
      s_startup_heading_deg = 0.0f;
    }
    Serial.printf("[FSM] TAKEOFF sequence active! Reference heading: %.2f deg\n", s_startup_heading_deg);
  }

  // 1. RC Stick Evaluation & Pilot Control Request Gating
  if(!rc_data.empty()){
    const RcChannels& ch = rc_data.front();
    if(ch.failsafe){
      Serial.println("[FSM] FAILSAFE in TAKEOFF! Triggering emergency stop...");
      fsm_trigger_emergency();
      evaluate_state_transitions();
      return;
    }

    uint16_t throttle = ch.channels[2];
    bool arm_switch_active = (ch.channel_count < 5) || (ch.channels[4] > RC_ARM_SWITCH_THRESHOLD_US);

    // Pilot abort: throttle lowered to minimum (< idle) AND arm switch disarmed before launch latch
    if(throttle < RC_THROTTLE_IDLE_US && !arm_switch_active && !s_pilot_control_latched){
      Serial.println("[FSM] Pilot aborted takeoff back to READY state.");
      s_takeoff_initialized = false;
      fsm_clear_event(EVT_TAKEOFF_INITIATED);
      fsm_set_event(EVT_TAKEOFF_ABORT);

      if(motor_cmd_queue != nullptr){
        MotorCommand cmd = MOTOR_CMD_DISARM;
        xQueueSend(motor_cmd_queue, &cmd, 0);
      }
      evaluate_state_transitions();
      return;
    }

    // Pilot Control Request Gating: 40% < throttle <= 80% (1400 - 1800 us)
    if(!s_pilot_control_latched){
      if(throttle >= RC_THROTTLE_LAUNCH_MIN_US && throttle <= RC_THROTTLE_LAUNCH_MAX_US){
        s_pilot_control_latched = true;
        s_takeoff_window_start_ms = millis();
        Serial.printf("[FSM] Pilot control request latched (throttle: %u us). Starting 5s safety window...\n", throttle);
      }
    }
  }

  // 2. Takeoff Safety Guards (Active once pilot control request is latched)
  if(s_pilot_control_latched){
    uint32_t now_ms = millis();

    // Guard A: Excessive Tilt (|roll| > 30 deg or |pitch| > 30 deg)
    if(!att_euler.empty()){
      Vec3<float> euler = att_euler.front();
      if(fabs(euler.x) > TAKEOFF_MAX_TILT_DEG || fabs(euler.y) > TAKEOFF_MAX_TILT_DEG){
        Serial.printf("[FSM] TAKEOFF SAFETY VIOLATION! Excessive tilt: Roll=%.1f deg, Pitch=%.1f deg\n", euler.x, euler.y);
        fsm_set_event(EVT_TAKEOFF_SAFETY_VIOLATION);
        fsm_trigger_emergency();
        s_takeoff_initialized = false;
        evaluate_state_transitions();
        return;
      }

      // Guard B: Heading Divergence (|Delta yaw| > 30 deg)
      float yaw_divergence = fabs(euler.z - s_startup_heading_deg);
      if(yaw_divergence > 180.0f) yaw_divergence = fabs(yaw_divergence - 360.0f);
      if(yaw_divergence > TAKEOFF_MAX_YAW_DIVERGENCE_DEG){
        Serial.printf("[FSM] TAKEOFF SAFETY VIOLATION! Heading divergence: DeltaYaw=%.1f deg\n", yaw_divergence);
        fsm_set_event(EVT_TAKEOFF_SAFETY_VIOLATION);
        fsm_trigger_emergency();
        s_takeoff_initialized = false;
        evaluate_state_transitions();
        return;
      }
    }

    // Guard C: Runaway / Anti-Flip Guard (|omega| > 150 deg/s)
    if(!gyro.empty()){
      Vec3<float> omega = gyro.front();
      if(fabs(omega.x) > TAKEOFF_MAX_ANGULAR_RATE_DPS ||
         fabs(omega.y) > TAKEOFF_MAX_ANGULAR_RATE_DPS ||
         fabs(omega.z) > TAKEOFF_MAX_ANGULAR_RATE_DPS){
        Serial.printf("[FSM] TAKEOFF SAFETY VIOLATION! Runaway angular rate: [%.1f, %.1f, %.1f] deg/s\n",
                      omega.x, omega.y, omega.z);
        fsm_set_event(EVT_TAKEOFF_SAFETY_VIOLATION);
        fsm_trigger_emergency();
        s_takeoff_initialized = false;
        evaluate_state_transitions();
        return;
      }
    }

    // 3. Verification Window Completion -> Transition to FLIGHT
    if((now_ms - s_takeoff_window_start_ms) >= TAKEOFF_VERIFY_DURATION_MS){
      Serial.println("[FSM] Takeoff 5s safety window satisfied! Transitioning to FLIGHT state.");
      s_takeoff_initialized = false;
      fsm_clear_event(EVT_TAKEOFF_INITIATED);
      fsm_set_event(EVT_TAKEOFF_COMPLETE);
    }
  }

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
  fsm_on_state_transition(UNINIT, UNINIT);
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
  if(app_task_handles[TASK_MOTOR_ID] != nullptr)
    xTaskNotify(app_task_handles[TASK_MOTOR_ID], NOTIFY_MOTOR_EMERGENCY_STOP, eSetBits);
  if(motor_cmd_queue != nullptr){
    MotorCommand cmd = MOTOR_CMD_EMERGENCY_STOP;
    xQueueSendToFront(motor_cmd_queue, &cmd, 0);
  }
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
      case TAKEOFF:     handle_state_takeoff();                 break;
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
