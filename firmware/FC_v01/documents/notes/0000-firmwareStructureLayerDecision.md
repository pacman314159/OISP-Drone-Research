---
id: "0000"
title: Firmware 5-Layer Architectural Decision & Directory Layout
author: Drone Research Team / Developer
date: 2026-09-15
time: 19:17:05 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#layers", "#freertos"]
references:
  - "[[Firmware Structure]]"
  - "[[System Architecture v01]]"
---

# Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout

## 1. Metadata & Context
- **ID**: `0000`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-15
- **Time**: 19:17:05 +07:00
- **Status**: Approved & Enforced

---

## 2. Core Architectural Decision
To guarantee high-performance real-time processing (500 Hz IMU flight loop), strict OSAL/middleware isolation, zero dynamic memory allocation during flight, and modular multi-target portability, the `FC_v01` firmware is structured into **5 distinct decoupled layers**.

```
+-------------------------------------------------------------+
| Layer 5: Application Layer (`src/app/`, `src/main.cpp`)     |
+-------------------------------------------------------------+
| Layer 4: Flight Core & Algorithms                           |
|          (`src/core/control/`, `src/core/estimators/`,      |
|           `src/core/math/`, `src/core/telemetry/`,          |
|           `src/core/daq/`)                                  |
+-------------------------------------------------------------+
| Layer 3: Middleware & Inter-Task IPC Layer                  |
|          (`src/middleware/`)                                |
+-------------------------------------------------------------+
| Layer 2: Drivers Layer                                      |
|          (`src/drivers/imu/`, `src/drivers/baro/`,          |
|           `src/drivers/rc_rx/`, `src/drivers/led/`)         |
+-------------------------------------------------------------+
| Layer 1: Platform HAL & Targets                             |
|          (`src/platforms/hal/`,                             |
|           `src/platforms/targets/espressif/`,               |
|           `src/platforms/targets/stm/`)                     |
+-------------------------------------------------------------+
```

---

## 3. Detailed Layer Specifications & Roadmap

### Layer 5: Application Layer
- **Directory**: `src/app/`, `src/main.cpp`
- **Responsibilities & Scope**: `src/main.cpp` serves strictly as the system entry point (`app_main()` / `setup()` & `loop()`). All finite state machine (FSM) state logic and FreeRTOS task spawning will be triggered from `src/app/`.

### Layer 4: Flight Core & Algorithms Layer
- **Directory**: `src/core/`
  - `src/core/daq/`: Data Acquisition (DAQ) task runners (`daq_tasks.cpp`, `daq_tasks.h`) for high-frequency sensor sampling loops.
  - `src/core/control/`: Cascaded PID rate/angle controllers, motor mixer.
  - `src/core/estimators/`: Complementary / Kalman filters for orientation & altitude estimation.
  - `src/core/math/`: Fast fixed-point / vector / quaternion mathematics libraries.
  - `src/core/telemetry/`: Telemetry transport and data packaging tasks.

### Layer 3: Middleware & Centralized Inter-Task IPC Layer
- **Directory**: `src/middleware/`
  - **Centralized Synchronization**: All inter-task communication primitives (FreeRTOS mutexes `i2c0_mutex`/`i2c1_mutex`, semaphores, queues, and static ring buffers) MUST be centralized inside Layer 3. Shared resource allocations and IPC declarations must never be scattered across driver or application code.
  - `mutexes.h`, `mutexes.cpp`: Global FreeRTOS bus synchronization handles.
  - `ring_buffer.h`: Thread-safe static template lock-free/critical section ring buffers.

### Layer 2: Sensor & Peripheral Drivers Layer
- **Directory**: `src/drivers/`
  - `src/drivers/imu/`: MPU6050, HMC5883L drivers under active development.
  - `src/drivers/baro/`: BMP180 driver under active development. BMP280 and MS5611 are dummy implementations reserved for future sensor development.
  - `src/drivers/rc_rx/`: FS-iA6B IBUS/PPM receiver drivers.
  - **Task Scoping Constraint**: All FreeRTOS task declarations inside `src/drivers/` are strictly forbidden; all task runners belong exclusively to Layer 4 (`src/core/daq/`, `src/core/control/`, `src/core/estimators/`, `src/core/telemetry/`) or Layer 5 (`src/app/`).

### Layer 1: Platform HAL & Hardware Target Drivers Layer
- **Directory**: `src/platforms/`
  - `src/platforms/hal/`: Abstract virtual interface declarations (`hal_i2c.h`, `hal_i2c_async.h`).
  - `src/platforms/targets/espressif/`: Active target under development (ESP32-S3 hardware I2C/SPI drivers).
  - `src/platforms/targets/stm/`: Dummy placeholder for future STM32 target development.

---

## 4. Documentation Roadmap
In the future, 3 primary technical documents will be developed simultaneously alongside the implementation process:
1. `System Life Cycle.md`
2. `RTOS Architecture.md`
3. `Firmware Structure.md`

*(Note: These 3 documents are omitted for now until active feature implementation begins.)*

---

## 5. Architectural Rules & Constraints
1. **Centralized Inter-Task IPC**: All shared mutexes, semaphores, queues, and ring buffers must be allocated and declared in Layer 3 (`src/middleware/`).
2. **Dependency Inversion & Downward Includes**: Higher layers may depend on lower layers, but lower layers must never import higher layers.
3. **Zero Dynamic Allocation**: No `malloc()`, `free()`, `new`, or `delete` within the core 500 Hz flight loop.
4. **Explicit Physical Units**: Standard SI units specified in code comments (`float accel; // m/s^2`, `float gyro; // rad/s`).
5. **Naming Conventions**: `snake_case` for variables & functions, `PascalCase` for classes & structs, `ALL_CAPS_SNAKE` for constants & enums.
