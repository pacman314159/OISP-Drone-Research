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
  TASK_ATTITUDE_FREQ_HZ = 250,
  TASK_MAG_DAQ_FREQ_HZ   = 75,
  TASK_BARO_DAQ_FREQ_HZ  = 50
};
```

### Rule 3: Complete Target Hardware Abstraction Across Layers 1 to 4
- No entity, peripheral driver, middleware module, algorithm, or application component across Layers 1 to 4 shall import target-specific headers (e.g. `<Wire.h>`, `esp_err.h`, or `stm32f4xx_hal.h`) or have any awareness of whether the underlying physical I2C or peripheral drivers are for STM32, ESP32, or any other target SoC.
- All hardware interactions MUST occur strictly through abstract HAL interfaces (`HAL_I2C`, `HAL_I2C_ASYNC`) and target hardware accessor functions (`get_i2c0_bus()`).

### Rule 4: Prohibition of `auto` Variable Declarations
- `auto` type deduction for variables is strictly forbidden across all firmware layers to prevent type ambiguity, hidden conversions, pointer/reference decay, and unintended value copies.
- Every variable MUST explicitly specify its concrete type (e.g., `MPU6050* mpu`, `TickType_t last_wake_time`, `uint8_t buffer[14]`).

### Rule 5: Deterministic Periodic Task Timing via `vTaskDelayUntil`
- All periodic DAQ tasks, estimation filters, and PID control loops MUST use `vTaskDelayUntil(&last_wake_time, period_ticks)` rather than `vTaskDelay()`.
- This eliminates loop execution phase drift and guarantees precise, deterministic release frequencies (e.g. exact 500 Hz / 2 ms periods for the IMU loop).

### Rule 6: Centralized Inter-Task Communication (`ipc.h` / `ipc.cpp`)
- All Layer 3 shared RTOS resources — including synchronization primitives (mutexes `i2c0_mutex`/`i2c1_mutex`, semaphores), inter-task data pipes (queues, ring buffers `imu_ring_buffer`), and shared memory allocations — MUST be declared centrally in `src/middleware/ipc.h` and instantiated in `src/middleware/ipc.cpp`.
- Initialization of all system IPC handles MUST be executed via `init_ipc()`. Scattering IPC declarations across drivers or application code is strictly forbidden.

### Rule 7: FreeRTOS Task Scoping & Scoped Layering
- FreeRTOS tasks MUST be declared and defined strictly in Layer 4 (`src/core/`, e.g. `src/core/daq/`, `src/core/control/`, `src/core/estimators/`, `src/core/telemetry/`) or Layer 5 (`src/app/`).
- Declaring or creating FreeRTOS tasks inside Layer 2 (`drivers/`) or below is strictly prohibited. `daq/` stands for **Data Acquisition**.

---

## 3. Impact & Architectural Constraints
1. **Priority Rule**: Verify all new RTOS task allocations reference `TaskPriorities` enum values (`[Note 0002]`).
2. **Config Enum Rule**: Declare task names, priorities, sizes, and frequencies in the 4 enum groups in `include/config.h`.
3. **HAL Isolation Rule**: Ensure driver and application code consume `HAL_I2C&` exclusively via `get_i2c0_bus()`.
4. **Explicit Typing Rule**: Replace any `auto` declarations with explicit types.
5. **Deterministic Delay Rule**: Use `vTaskDelayUntil()` for all periodic loops.
6. **Centralized IPC Rule**: Declare all shared locks, semaphores, queues, and ring buffers inside `src/middleware/ipc.h` / `ipc.cpp` and initialize via `init_ipc()`.
7. **Task Scoping Rule**: Ensure all task functions reside exclusively in `core/` or `app/` (e.g. `src/core/daq/daq_tasks.h`).
