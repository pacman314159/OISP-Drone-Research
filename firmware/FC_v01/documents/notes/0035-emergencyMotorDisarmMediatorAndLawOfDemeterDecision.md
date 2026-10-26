---
id: "0035"
title: Emergency Motor Disarm, Mediator Orchestration & Law of Demeter Architectural Decision
author: Drone Research Team / Developer
date: 2026-10-23
time: 18:35:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#fsm", "#motor", "#actuator", "#safety", "#freertos", "#mediator", "#law_of_demeter"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
  - "[[0034-motorDriverLedcHalQuadxActuatorTaskArchitecture]]"
---

# Zettel 0035: Emergency Motor Disarm, Mediator Orchestration & Law of Demeter Architectural Decision

## 1. Metadata & Scope
- **ID**: `0035`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-23
- **Time**: 18:35:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0020]`, `[Note 0031]`, `[Note 0034]`

---

## 2. Problem Statement & Architectural Tension

Following the initial scaffolding of the 250 Hz actuator subsystem (`[Note 0034]`), an architectural tension emerged regarding arming authority and peripheral manipulation:
1. **The Law of Demeter & Layer Isolation**:
   In the 5-layer architecture (`[Note 0000]`), peripheral drivers (Layer 2) are instantiated and passed directly to their owning FreeRTOS tasks in the Core Layer (Layer 4) via `void* arg` (`[Note 0006]`). Direct access or mutation of peripheral drivers from the Application Layer (Layer 5, `fsm.cpp` or `app.cpp`) violates layer decoupling and introduces cross-core race conditions between Core 0 (FSM 100 Hz) and Core 1 (Motor 250 Hz).
2. **Safety Authority Scoping**:
   Actuator task runners (`motor_task`) must **never** possess independent authority to decide *when* to arm or disarm the drone. Arming authority belongs exclusively to the supervisor in the Application Layer (`fsm_task` and `app_start()`).
3. **Sub-Microsecond Emergency Cutoff**:
   In catastrophic scenarios (e.g., ground tip-over due to motor multiplexing error, failsafe trigger, crash detection, or tilt $> 60^\circ$), motor PWM output must be killed instantly without waiting for queue processing or the expiration of the 250 Hz periodic delay ($4\,\text{ms}$).

---

## 3. Core Architectural Decisions

### Rule 1: Application Layer Exclusive Arming Authority
- The sole authority to transition between disarmed and armed states belongs strictly to Layer 5 (`fsm_task` via FSM transition rules and cold boot setup in `app_start()`).
- Layer 4 `motor_task` is a purely reactive consumer. It checks whether the actuator is armed; if disarmed, it suppresses throttle actuation and holds motors at $1000\,\mu\text{s}$ (`ESC_PWM_DISARMED_US`).

### Rule 2: Layer 2 `MotorDriver` Internal Defense-in-Depth
- `MotorDriver` encapsulates its own internal safety flag `bool _armed{false}`:
  - `arm_all()` sets `_armed = true`.
  - `disarm_all()` sets `_armed = false` and forces all channels to `ESC_PWM_DISARMED_US` ($1000\,\mu\text{s}$, 25% duty).
  - `is_armed() const` exposes current armed state.
- **Fail-Safe Output Interlock**: In `set_motor_normalized()`, execution is strictly gated:
  ```cpp
  if(!_initialized || !_armed || motor_idx >= MOTOR_COUNT) return;
  ```
  Even if upstream mixer code mistakenly calculates throttle while disarmed, `MotorDriver` internally rejects pulse generation.
- **Mandatory Cold-Boot Disarm**: `app_start()` unconditionally calls `motor_driver.disarm_all()` immediately following initialization.

### Rule 3: Dual-Channel Mediator Architecture (Command Queue + Out-of-Band Notification)
To adhere to the Law of Demeter without incurring emergency latency, communication from Layer 5 to Layer 4 is decoupled across two dedicated channels in Layer 3 IPC:

```
[ Layer 5: FSM Supervisor ]
        │
        ├──> (Routine Arm/Disarm) ────> [ Layer 3: motor_cmd_queue ] ────> [ Layer 4: motor_task ]
        │                                                                            │
        └──> (Emergency Cutoff)   ────> [ FreeRTOS Direct Notification ] ───────────┘
                                        (NOTIFY_MOTOR_EMERGENCY_STOP)
```

1. **Channel A: Command Queue (`motor_cmd_queue`) for Lifecycle Transitions**:
   - Queue depth: `MOTOR_CMD_QUEUE_SIZE = 4` items of `enum MotorCommand`:
     - `MOTOR_CMD_DISARM = 0`
     - `MOTOR_CMD_ARM = 1`
     - `MOTOR_CMD_EMERGENCY_STOP = 2`
   - Allocated statically in `src/middleware/ipc.cpp`.
2. **Channel B: Out-of-Band Preemptive Emergency Signal (`xTaskNotify`)**:
   - Carried directly via 32-bit FreeRTOS task notification bits:
     ```cpp
     enum MotorNotificationBits : uint32_t {
       NOTIFY_MOTOR_NONE           = 0,
       NOTIFY_MOTOR_EMERGENCY_STOP = (1 << 0)
     };
     ```
   - When emergency cutoff is triggered:
     ```cpp
     xTaskNotify(app_task_handles[TASK_MOTOR_ID], NOTIFY_MOTOR_EMERGENCY_STOP, eSetBits);
     ```

### Rule 4: Two-Stage Verification in `motor_task` ("The Second Check")
Inside `motor_task`, emergency inspection is prioritized at the very top of each execution cycle:
1. **1st Check (Delivery Check)**: Non-blocking inspection of pending task notifications (`xTaskNotifyWait(0, ULONG_MAX, &notify_bits, 0) == pdTRUE`).
2. **2nd Check (Signal Verification)**: Explicitly tests whether the received notification is specifically an emergency stop command:
   ```cpp
   if((notify_bits & NOTIFY_MOTOR_EMERGENCY_STOP) != 0){
     motor->disarm_all();
     // Purge command queue to discard stale arming commands
     MotorCommand dummy;
     while(xQueueReceive(motor_cmd_queue, &dummy, 0) == pdTRUE);
     SYSVIEW_END(task_id);
     continue;
   }
   ```
   This prevents arbitrary non-emergency notifications (e.g., future sync or telemetry pulses) from inadvertently shutting down flight motors.

### Rule 5: Actuator Task Priority Elevation to Realtime (`4`)
In [`include/config.h`](include/config.h), `TASK_MOTOR_PRIORITY` is elevated to priority **4** (Realtime), co-equal with high-frequency DAQ pipelines and exceeding FSM (3) and telemetry (1..2). This ensures motor mixing and disarm preemption execute immediately without scheduler starvation.

---

## 4. Cross-References & Traceability
- [[0000-firmwareStructureLayerDecision]]: Layer 1-5 structural hierarchy and encapsulation.
- [[0001-taskConfigMiddlewareIsrDecision]]: Centralized IPC allocation in `src/middleware/ipc.*`.
- [[0002-taskEnumsAndHalAbstractionDecision]]: Centralized task parameters in `include/config.h`.
- [[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]: Strict driver passing to task runners.
- [[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]: Centralized state transitions and arming gating.
- [[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]: Pre-flight 6-gate lockout.
- [[0034-motorDriverLedcHalQuadxActuatorTaskArchitecture]]: Normalized motor driver and LEDC HAL.

