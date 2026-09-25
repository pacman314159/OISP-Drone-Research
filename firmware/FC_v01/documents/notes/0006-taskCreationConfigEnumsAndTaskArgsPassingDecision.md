---
id: "0006"
title: Centralized Task Parameter Enums, Mandatory TaskIDs & Direct Driver Pointer Argument Passing Rule
author: Drone Research Team / Developer
date: 2026-09-22
time: 18:41:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#freertos", "#config", "#task_ids", "#sysview"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
---

# Zettel 0006: Centralized Task Parameter Enums, Mandatory TaskIDs & Direct Driver Pointer Argument Passing Rule

## 1. Metadata & Context
- **ID**: `0006`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-22
- **Time**: 18:41:00 +07:00
- **Status**: Approved & Enforced
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`

---

## 2. Core Architectural Decisions

### Rule 1: Centralized Enum Task Parameter Declarations (`include/config.h`)
All FreeRTOS task creation parameters MUST be declared centrally inside `include/config.h` using structured enum and constant groups:
1. `TaskNames`: Constant string task identifiers (`TASK_*_NAME`)
2. `TaskIDs`: System-wide unique task enumeration (`enum TaskIDs : uint32_t`)
3. `TaskCores`: Hardware CPU core affinity assignments (`enum TaskCores : int32_t`)
4. `TaskPriorities`: 4-level RTOS priority assignments (`enum TaskPriorities : UBaseType_t`)
5. `TaskSizesBytes`: Memory stack size allocations (`enum TaskSizesBytes : uint32_t`)
6. `TaskFrequenciesHz`: Deterministic release loop frequencies (`enum TaskFrequenciesHz : uint32_t`)

Hardcoding magic numbers or string literals for task stack sizes, priorities, names, or core IDs inside task creation calls is strictly prohibited.

### Rule 2: Direct Driver Pointer Argument Passing & `config.h` TaskID Inclusion
- Every task defined in `FC_v01` MUST be assigned a unique `TaskIDs` enum value inside `include/config.h`.
- Task spawning in Layer 5 (`src/app/`) passes target driver pointers (or service options) directly via the `void* arg` parameter of `xTaskCreatePinnedToCore()`.
- Wrapper structures such as `TaskArgs<T>` are eliminated; task runners import `include/config.h` directly to access their assigned `TASK_*_ID` constants.

```cpp
// Example in Layer 5 (src/app/app.cpp):
xTaskCreatePinnedToCore(
  accel_gyro_daq_task,
  TASK_ACCEL_GYRO_DAQ_NAME,
  TASK_ACCEL_GYRO_DAQ_STACK_SIZE,
  &mpu,
  TASK_ACCEL_GYRO_DAQ_PRIORITY,
  nullptr,
  TASK_ACCEL_GYRO_DAQ_CORE
);
```

### Rule 3: Task Runner Direct Config Access & System Identification Responsibility
- Each task runner function in Layer 4 (`src/core/`) accesses its assigned `task_id` constant directly from `include/config.h` (e.g., `const uint32_t task_id = TASK_ACCEL_GYRO_DAQ_ID;`).
- The task runner extracts its driver instance pointer by casting `*arg` (`MPU6050* mpu = static_cast<MPU6050*>(arg);`).
- The task runner uses `task_id` for internal system identification, telemetry metadata, status reporting, and SystemView event tracing (`SYSVIEW_START(task_id)` / `SYSVIEW_END(task_id)`).

```cpp
// Example in Layer 4 Task Runner (src/core/daq/daq_tasks.cpp):
void accel_gyro_daq_task(void* arg){
  if(arg == nullptr) return;

  MPU6050* mpu = static_cast<MPU6050*>(arg);
  const uint32_t task_id = TASK_ACCEL_GYRO_DAQ_ID;

  while(true){
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id);
    // ... task execution ...
    SYSVIEW_END(task_id);
  }
}
```

---

## 3. Impact & Architectural Constraints
1. **Config Enum Rule**: Declare task names, IDs, priorities, stack sizes, frequencies, and core IDs in `include/config.h`.
2. **Direct Argument Rule**: Pass driver instance pointers directly via `void* arg` when spawning tasks in Layer 5.
3. **Direct TaskID Access Rule**: Task functions in Layer 4 access `TASK_*_ID` directly from `include/config.h` and pass it to SystemView markers (`SYSVIEW_START`/`SYSVIEW_END`).
