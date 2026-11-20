#pragma once
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "middleware/flight_events.h"

// =============================================================================
// SYSTEM FLIGHT STATES
// =============================================================================
enum SystemState : uint16_t {
  // Normal expected flight states
  UNINIT = 0,
  CALIB,
  PREFLIGHT,
  READY,
  TAKEOFF,
  FLIGHT,
  LANDING,    // [TEMPORARY NOTE] Currently bypassed; direct FLIGHT -> POSTFLIGHT transition on 3s hold
  POSTFLIGHT,
  DEVELOPMENT,

  // Failure / Emergency flight states
  LEVELOUT,
  CRASHED,
  FREEFALL
};

// =============================================================================
// SYSTEM EVENT MASKS (EVT_ prefix)
// =============================================================================
// Moved to middleware/flight_events.h (C5): Layer 4 reports events through a
// Layer 3 contract instead of importing this header. Included above.

// =============================================================================
// TRANSITION RULE MATCH POLICY
// =============================================================================
enum MatchPolicy : uint8_t {
  ALL = 0,
  ANY,
  ALWAYS,
  NEVER
};

// =============================================================================
// TRANSITION RULE STRUCTURE
// =============================================================================
struct TransitRule {
  SystemState from_state;
  SystemState to_state;

  uint32_t trigger_mask;
  uint32_t neg_trigger_mask;
  MatchPolicy trigger_policy;

  uint32_t blocker_mask;
  MatchPolicy block_policy;

  const char* rule_name;
};

// =============================================================================
// FSM PUBLIC API
// =============================================================================
void fsm_init();
void fsm_task(void* arg);
SystemState fsm_get_state();
uint32_t fsm_get_events();
// fsm_set_event() / fsm_clear_event() are declared in middleware/flight_events.h
// so Layer 4 can report events without importing this header (C5).
void fsm_trigger_emergency();
bool fsm_is_armed_allowed();
