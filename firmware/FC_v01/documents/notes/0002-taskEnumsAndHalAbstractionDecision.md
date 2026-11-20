---
id: "0002"
title: System 4 Priority Levels, Enum Task Config, Target-Agnostic HAL, Explicit Typing, Deterministic Delays & Centralized IPC
author: Drone Research Team / Developer
date: 2026-09-15
time: 20:48:49 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#freertos", "#config", "#hal", "#ipc"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0038-osalLayer3IpcDecouplingAndInterfaceTasksConsolidation]]"
---

# Zettel 0002: System 4 Priority Levels, Enum Task Config, Target-Agnostic HAL, Explicit Typing, Deterministic Delays & Centralized IPC

## 1. Metadata & Context
- **ID**: `0002`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-15
- **Time**: 20:48:49 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0001]`

---

## 2. Core Architectural Decisions

### Rule 1: Strict 4-Level RTOS Priority Architecture (`enum TaskPriorities`)
The firmware enforces exactly **4 system-wide priority levels** across all FreeRTOS tasks to prevent priority inversion, scheduler fragmentation, and arbitrary priority assignment:
1. `TASK_PRIORITY_LOW` (Telemetry, Housekeeping)
2. `TASK_PRIORITY_MED` (Barometer, Magnetometer DAQ)
3. `TASK_PRIORITY_HIGH` (Attitude Rate & Control Loops)
4. `TASK_PRIORITY_REALTIME` (500 Hz IMU DAQ Loop)

Assigning numerical priority literals outside of `TaskPriorities` is strictly prohibited across all layers.

### Rule 2: Structured Enum-Based Task Parameters (`include/config.h`)
All RTOS task configurations — including task names, priority levels, loop frequencies, stack sizes, and task groups — MUST be declared centrally inside `include/config.h` using 4 structured enum groups:
- `TaskPriorities`
- `TaskNames`
- `TaskSizes`
- `TaskFrequencies`

Example structure in `include/config.h`:
```cpp
enum TaskPriorities : uint8_t {
  TASK_PRIORITY_LOW      = 1,
  TASK_PRIORITY_MED      = 2,
  TASK_PRIORITY_HIGH     = 3,
  TASK_PRIORITY_REALTIME = 4
};

enum TaskFrequencies : uint16_t {
  TASK_IMU_DAQ_FREQ_HZ  = 500,
  TASK_MAG_DAQ_FREQ_HZ  = 75,
  TASK_BARO_DAQ_FREQ_HZ = 50
};
```

### Rule 3: Hardware Access Through HAL Interfaces
- All hardware interactions MUST occur strictly through abstract HAL interfaces (`HAL_I2C`, `HAL_I2C_ASYNC`, `HAL_PulseTX`) and target hardware accessor functions (`get_i2c0_bus()`, `get_pulse_tx0()`).
- No public header or function signature in Layers 2 through 5 may expose a target-specific type. Target-neutral value types are mandatory at every public API — the button event API is `uint8_t` (`ipc_pop_btn_events()` in `src/middleware/ipc.h`).
- **Header-import prohibition retired (2026-11-18, Gate A — owner ruling).** The former blanket rule — "No entity, peripheral driver, middleware module, algorithm, or application component across Layers 1 to 4 shall import target-specific headers (e.g. `<Wire.h>`, `<driver/rmt.h>`, `esp_err.h`, or `stm32f4xx_hal.h`) or have any awareness of whether the underlying physical I2C or peripheral drivers are for STM32, ESP32, or any other target SoC" — is deleted. It was unsatisfiable as written: it also forbade Layer 1 (`src/platforms/targets/`), the layer whose function is to own target headers, and it directly contradicted `[Note 0001]`, which requires hardware ISR bodies in Layer 3. This retirement supersedes `0002` §61 and the identical restriction in `documents/Firmware Structure.md` §1.4.
- **ISR ownership is unaffected by this retirement.** Hardware ISR bodies and their GPIO registration still belong to Layer 3 (`src/middleware/ipc.cpp`), per `[Note 0001]`; debounce, 3-second hold classification, the event bitmask and the FreeRTOS notification are also Layer 3, exposed through the target-neutral `uint8_t` API. Declaring raw ISRs in `src/app/` or `src/drivers/` remains forbidden.
- **Recorded debt.** `src/core/interface/user_button.cpp` (Layer 4) still performs GPIO configuration and level reads directly; it is technical debt against a future `HAL_GPIO` abstraction, which is deferred (do not describe it as approved work).

### Rule 4: Prohibition of `auto` Variable Declarations
- `auto` type deduction for variables is strictly forbidden across all firmware layers to prevent type ambiguity, hidden conversions, pointer/reference decay, and unintended value copies.
- Every variable MUST explicitly specify its concrete type (e.g., `MPU6050* mpu`, `TickType_t last_wake_time`, `uint8_t buffer[14]`).

### Rule 5: Deterministic Periodic Task Timing via `vTaskDelayUntil`
- All periodic DAQ tasks, estimation filters, and PID control loops MUST use `vTaskDelayUntil(&last_wake_time, period_ticks)` rather than `vTaskDelay()`.
- This eliminates loop execution phase drift and guarantees precise, deterministic release frequencies (e.g. exact 500 Hz / 2 ms periods for the IMU loop).

### Rule 6: Centralized Inter-Task Communication (`ipc.h` / `ipc.cpp`)
- **Ownership.** Every RTOS object with more than one consumer (queue, mutex, semaphore, ring buffer) MUST be declared in `src/middleware/` and created in `src/middleware/ipc.cpp`. No module creates its own.
- **Types.** Any type that crosses an IPC boundary MUST be a POD declared in `include/ipc_types.h`, containing no OS type and no vendor type.
- **Access.** Queues, mutexes and the task registry MUST be accessed through the Layer 3 API (`ipc_*`). Task registry mutations go through `ipc_register_task()` / `ipc_get_task()`; only Layer 5's task-creation site in `app.cpp` may populate a slot directly at creation time. Single-producer/single-consumer ring buffers MAY be accessed directly, and their SPSC discipline MUST be stated at the declaration (`middleware/ipc_handles.h`).
- *(C15, 2026-11-19: replaces the blanket "everything through `ipc.h`" wording. The headers are now split — `include/ipc_types.h`, `middleware/ipc_handles.h`, `middleware/ipc_lifecycle.h`, with `middleware/ipc.h` kept as an umbrella — while `ipc.cpp` remains the single owner.)*

### Rule 7: FreeRTOS Task Scoping & Scoped Layering
- FreeRTOS tasks MUST be declared and defined strictly in Layer 4 (`src/core/`, e.g. `src/core/sensor/`, `src/core/control/`, `src/core/actuator/`, `src/core/math/`, `src/core/safety/`, `src/core/telemetry/`, `src/core/blackbox/`, `src/core/interface/`) or Layer 5 (`src/app/`).
- Declaring or creating FreeRTOS tasks inside Layer 2 (`drivers/`) or below is strictly prohibited. `sensor/` houses high-frequency sensor Data Acquisition and calibration routines [Note 0016]; `actuator/` houses motor/ESC activation and mixing task runners; `safety/` houses pre-flight gate evaluation and safety-status aggregation.

---

## 3. Impact & Architectural Constraints
1. **Priority Rule**: Verify all new RTOS task allocations reference `TaskPriorities` enum values (`[Note 0002]`).
2. **Config Enum Rule**: Declare task names, priorities, sizes, and frequencies in the 4 enum groups in `include/config.h`.
3. **HAL Isolation Rule**: Ensure driver and application code consume `HAL_I2C&` exclusively via `get_i2c0_bus()`.
4. **Explicit Typing Rule**: Replace any `auto` declarations with explicit types.
5. **Deterministic Delay Rule**: Use `vTaskDelayUntil()` for all periodic loops.
6. **Centralized IPC Rule**: Declare all shared locks, semaphores, queues, and ring buffers in `src/middleware/` and create them in `src/middleware/ipc.cpp`; initialize via `init_ipc()`. IPC contracts live in `include/ipc_types.h`. Queues, mutexes and the task registry go through the `ipc_*` API; SPSC ring buffers may be accessed directly with their discipline stated at the declaration (Rule 6, C15).
7. **Task Scoping Rule**: Ensure all task functions reside exclusively in `core/` or `app/` (e.g. `src/core/sensor/daq_tasks.h`, `src/core/sensor/sensor_preprocess_tasks.h`, `src/core/telemetry/ble_telem_tasks.h`).
