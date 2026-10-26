---
id: "0034"
title: Motor Driver LEDC HAL Abstraction, Normalized [0.0, 1.0] PWM Mapping & Actuator Task Architecture
author: Drone Research Team / Developer
date: 2026-10-23
time: 16:33:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#hal", "#ledc", "#pwm", "#motor", "#actuator", "#freertos", "#ipc"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
---

# Zettel 0034: Motor Driver LEDC HAL Abstraction, Normalized [0.0, 1.0] PWM Mapping & Actuator Task Architecture

## 1. Metadata & Scope
- **ID**: `0034`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-23
- **Time**: 16:33:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0007]`, `[Note 0016]`, `[Note 0031]`

---

## 2. Problem Statement & Motivation
Following the completion of the 6-gate preflight check suite (`[Note 0031]`) and the 7-state quaternion EKF estimator (`[Note 0032]`), `FC_v01` lacked physical actuator control interfaces. To drive quadrotor Electronic Speed Controllers (ESCs):
1. **Target Hardware Decoupling**: Hardware PWM pulse generation must not couple upper flight algorithms directly to ESP32-S3 register or SDK headers.
2. **Standard ESC Protocol Calibration**: Standard ESCs operate with 250 Hz carrier frequency ($T = 4000\,\mu\text{s}$) where active throttle spans $1000\,\mu\text{s}$ to $2000\,\mu\text{s}$ (25% to 50% physical duty cycle). Higher flight controllers (such as the upcoming INDI controller) must remain purely dimensionless, operating strictly over normalized $[0.0, 1.0]$ commands.
3. **Task Domain Organization**: Motor mixing and ESC output must reside in a dedicated Layer 4 domain (`src/core/actuator/motor_tasks.cpp/.h`), consuming control signals via low-latency critical sections (`SYS_ENTER_CRITICAL()`).

---

## 3. Core Architectural Decisions

### Rule 1: Layer 1 Platform HAL Abstraction (`HAL_PWM` & `ESP32_PWM_LEDC`)
- Pure abstract interface `HAL_PWM` declared in `src/platforms/hal/hal_pwm.h` defines target-agnostic timer configuration, channel binding, pulse width generation (`set_pulse_us`), and channel shutdown.
- Physical target driver `ESP32_PWM_LEDC` in `src/platforms/targets/espressif/pwm_ledc.cpp/.h` wraps ESP-IDF `<driver/ledc.h>` on `LEDC_LOW_SPEED_MODE` and `LEDC_TIMER_0` at 14-bit resolution (`ESC_LEDC_TIMER_BIT_DEPTH = 14`, 16384 ticks).
- Dynamically converts microsecond pulse widths to timer ticks via 64-bit integer arithmetic:
  $$\text{ticks} = \frac{\text{pulse\_us} \times f_{\text{pwm}} \times 2^{\text{res\_bits}}}{10^6}$$
  At 250 Hz ($T = 4000\,\mu\text{s}$), $1000\,\mu\text{s}$ (25% duty) maps to $4096$ ticks and $2000\,\mu\text{s}$ (50% duty) maps to $8192$ ticks, yielding 4096 discrete throttle steps ($\sim 0.244\,\mu\text{s}$ per step).

### Rule 2: Layer 2 Motor Peripheral Driver (`MotorDriver`)
- Located in `src/drivers/motor/motor_driver.cpp/.h`.
- Operates strictly in the normalized domain $[0.0, 1.0]$:
  - Sanitizes and clamps input: inputs $< 0.0$ or non-finite are clamped to $0.0$; inputs $> 1.0$ are clamped to $1.0$.
  - Linearly maps $[0.0, 1.0] \rightarrow [1000\,\mu\text{s}, 2000\,\mu\text{s}]$.
- Exposes `disarm_all()` to immediately force all motor outputs to `ESC_PWM_DISARMED_US` ($1000\,\mu\text{s}$, 25% duty).

### Rule 3: Layer 3 Inter-Task IPC (`IndiControllerOutput`)
- Declared in `src/middleware/ipc.h` and allocated in `src/middleware/ipc.cpp`:
  ```cpp
  struct IndiControllerOutput {
    float       thrust_cmd{0.0f};          // Normalized collective throttle [0.0, 1.0]
    Vec3<float> u_in{0.0f, 0.0f, 0.0f};    // Normalized 3-axis torque moments [p, q, r] in [-1.0, 1.0]
    uint32_t    timestamp_us{0};           // Update timestamp (us)
    bool        is_armed{false};           // Armed status
  };
  extern IndiControllerOutput indi_output;
  ```
- Inter-task thread safety between the flight controller and actuator runner is guaranteed via `SYS_ENTER_CRITICAL()` and `SYS_EXIT_CRITICAL()`.

### Rule 4: Layer 4 Actuator Domain (`src/core/actuator/motor_tasks.cpp/.h`)
- Declares `void motor_task(void* arg)` executing deterministically at 250 Hz (Priority 3, `TASK_MOTOR_ID`).
- Receives driver instance via `MotorDriver* motor = static_cast<MotorDriver*>(arg)` per Note 0006.
- Encapsulates the execution loop with `SYSVIEW_START(TASK_MOTOR_ID)` and `SYSVIEW_END(TASK_MOTOR_ID)`, anti-windup overrun guard, and documented architectural stub for future INDI Quad-X mixing and attitude-priority saturation capping.

---

## 4. Cross-References & Traceability
- [[0000-firmwareStructureLayerDecision]]: Layer 1 HAL, Layer 2 Driver, Layer 3 IPC, and Layer 4 Core decoupling.
- [[0001-taskConfigMiddlewareIsrDecision]]: Centralized IPC allocation in `src/middleware/ipc.*`.
- [[0002-taskEnumsAndHalAbstractionDecision]]: Target-agnostic HAL and centralized task enums in `include/config.h`.
- [[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]: Passing `MotorDriver*` to `motor_task` via `void* arg`.
- [[0007-systemViewTracingAndRTOSTimingAnalysisConventions]]: SEGGER SystemView trace markers and 250 Hz overrun guards.
- [[0016-coreDirectoryCompactionAndDomainConsolidation]]: Scoping actuator tasks in `src/core/actuator/`.
- [[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]: Safe motor disarm lockout during preflight gating.

