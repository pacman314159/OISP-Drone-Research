#pragma once
#include "ipc_types.h"                  // POD contracts (neutral zone)
#include "middleware/ipc_handles.h"     // shared handles & ring buffers
#include "middleware/ipc_lifecycle.h"   // init / on-demand create / task registry

// =============================================================================
// LAYER 3 MIDDLEWARE & CENTRALIZED INTER-TASK IPC — UMBRELLA HEADER
// =============================================================================
// One owner (ipc.cpp), three declaration headers:
//   include/ipc_types.h          — POD contracts that cross IPC boundaries
//   middleware/ipc_handles.h     — extern handles, queues, ring buffers
//   middleware/ipc_lifecycle.h   — init, on-demand creation, task registry API
// New code should include the narrowest header it needs.

// Hardware Button Event Flags & Middleware ISR Handler ([Note 0001])
enum BtnEventMasks : uint8_t {
  BTN_EVT_NONE        = 0,
  BTN_EVT_SHORT_CLICK = (1 << 0),
  BTN_EVT_LONG_HOLD   = (1 << 1)
};

uint8_t ipc_pop_btn_events();
void user_btn_isr();

// INDI Controller Shared Output Signal (Producer: Control Task, Consumer: Motor Task via SYS_ENTER_CRITICAL)
struct IndiControllerOutput {
  float       thrust_cmd{0.0f};          // Normalized collective throttle command [0.0, 1.0]
  Vec3<float> u_in{0.0f, 0.0f, 0.0f};    // Normalized 3-axis torque commands [roll (p), pitch (q), yaw (r)] in [-1.0, 1.0]
  uint32_t    timestamp_us{0};           // Update timestamp in microseconds (us)
  bool        is_armed{false};           // Safety arming flag
};

extern IndiControllerOutput indi_output;
