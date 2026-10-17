---
title: FC_v01 Firmware Architecture & Structure Specification
author: Drone Research Team / Developer
date: 2026-09-18
version: 1.0.0
status: Approved & Enforced
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
  - "[[0004-simdMultiTypeSupportAndBuildIntegration]]"
---

# FC_v01 Firmware Architecture & Structure Specification

## 1. Executive Summary & Architectural Philosophy

The `FC_v01` flight controller firmware is designed for hard real-time, deterministic quadrotor flight control. Built on FreeRTOS and optimized for target microcontrollers (such as the Espressif ESP32-S3), the system guarantees predictable loop execution, strict OSAL/middleware isolation, target-agnostic physical hardware abstraction, and hardware-accelerated SIMD math computation.

### Core Architectural Principles
1. **Strict 500 Hz IMU Flight Loop Rate**: Primary sensor data acquisition (DAQ) and cascaded PID control loops operate at deterministic intervals (2.0 ms execution budget).
2. **Zero Dynamic Memory Allocation During Flight**: All RTOS tasks, ring buffers, queues, semaphores, and math structures are allocated statically during initialization. Zero `malloc()`, `free()`, `new`, or `delete` calls are permitted inside the flight execution loop.
3. **Strict Downward Dependency Hierarchy**: Higher application and flight core layers may consume lower layer abstractions, but lower layers (drivers, middleware, HAL) must never import or depend on higher layers.
4. **Target Hardware Isolation**: Driver, middleware, and core math modules across Layers 2 through 5 are entirely target-agnostic and consume standard HAL interfaces (`HAL_I2C`, `hal_simd`). Target-specific SDK headers (`<Wire.h>`, `esp_err.h`, `stm32f4xx_hal.h`) are restricted exclusively to Layer 1 target modules (`src/platforms/targets/`).

---

## 2. 5-Layer Firmware Architecture & Directory Layout

The codebase is organized into **5 decoupled structural layers**:

```
+-------------------------------------------------------------+
| Layer 5: Application Layer (`src/app/`, `src/main.cpp`)     |
+-------------------------------------------------------------+
| Layer 4: Flight Core & Algorithms                           |
|          (`src/core/sensor/`, `src/core/control/`,          |
|           `src/core/math/`, `src/core/telemetry/`,          |
|           `src/core/blackbox/`, `src/core/interface/`)      |
+-------------------------------------------------------------+
| Layer 3: Middleware & Inter-Task IPC Layer                  |
|          (`src/middleware/`)                                |
+-------------------------------------------------------------+
| Layer 2: Sensor & Peripheral Drivers Layer                  |
|          (`src/drivers/imu/`, `src/drivers/baro/`,          |
|           `src/drivers/rc_rx/`, `src/drivers/led/`)         |
+-------------------------------------------------------------+
| Layer 1: Platform HAL & Hardware Target Drivers             |
|          (`src/platforms/hal/`,                             |
|           `src/platforms/targets/espressif/`,               |
|           `src/platforms/targets/stm/`)                     |
+-------------------------------------------------------------+
```

### Layer Specifications & Directory Mapping

#### Layer 5: Application Layer
- **Directory**: [`src/app/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/app), [`src/main.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/main.cpp)
- **Role**: Entry point (`app_main()` / `setup()` & `loop()`) and high-level Finite State Machine (FSM) control (`app.cpp`, `fsm.cpp`). Triggers system initialization, IPC handle creation, and FreeRTOS task spawning.

#### Layer 4: Flight Core & Algorithms Layer
- **Directory**: [`src/core/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core)
  - `src/core/sensor/`: Unified sensor Data Acquisition (DAQ) sampling loops, sensor preprocessing loops (calibration application and noise filtering), and sensor calibration routines ([`daq_tasks.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/sensor/daq_tasks.cpp), [`sensor_preprocess_tasks.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/sensor/sensor_preprocess_tasks.cpp), [`calib.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/sensor/calib.cpp), [`calib_tasks.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/sensor/calib_tasks.cpp)).
  - `src/core/control/`: Cascaded PID rate and attitude angle controllers, motor mixer ([`control_tasks.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/control/control_tasks.cpp)).
  - `src/core/math/`: Linear algebra matrix engine ([`matrix.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/math/matrix.h)), 1st-order discrete low-pass filter algorithms ([`lpf.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/math/lpf.h)), 3D ellipsoid fitting ([`ellipsoid_fit.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/math/ellipsoid_fit.h)), and state estimators ([`estimator_tasks.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/math/estimator_tasks.cpp)).
  - `src/core/telemetry/`: Telemetry streaming protocols and wireless OTA update management ([`ble_telem_tasks.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/telemetry/ble_telem_tasks.cpp), [`ble_ota_tasks.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/telemetry/ble_ota_tasks.cpp)).
  - `src/core/blackbox/`: High-frequency flight data storage and SD card snapshot logger ([`sd_logger_task.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/blackbox/sd_logger_task.cpp), [`sd_logger_task.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/blackbox/sd_logger_task.h)).
  - `src/core/interface/`: Human-machine and status indication abstractions (reserved for user buttons and status LEDs).

#### Layer 3: Middleware & Centralized Inter-Task IPC Layer
- **Directory**: [`src/middleware/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware)
  - Centralizes all inter-task communication primitives, hardware ISR routing, and lock-free thread-safe queues ([`ipc.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/ipc.h), [`ipc.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/ipc.cpp)).
  - Contains thread-safe lock-free static ring buffers ([`ring_buffer.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/ring_buffer.h)) and critical section macros ([`critical_section.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/critical_section.h)).

#### Layer 2: Sensor & Peripheral Drivers Layer
- **Directory**: [`src/drivers/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers)
  - Sensor-specific drivers: IMU ([`mpu6050`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/imu/mpu6050.h), [`hmc5883l`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/imu/hmc5883l.h)), Barometer ([`bmp180`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/baro/bmp180.h), `bmp280`, `ms5611`), RC RX ([`rc_rx`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/rc_rx)).
  - Hardware peripheral drivers are strictly stateless/class-based drivers; FreeRTOS task creation is forbidden inside Layer 2.

#### Layer 1: Platform HAL & Hardware Target Drivers Layer
- **Directory**: [`src/platforms/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms)
  - `src/platforms/hal/`: Target-agnostic C++ pure abstract interfaces ([`hal_i2c.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/hal/hal_i2c.h), [`hal_i2c_async.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/hal/hal_i2c_async.h), [`hal_simd.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/hal/hal_simd.h)).
  - `src/platforms/targets/espressif/`: Physical ESP32-S3 implementations (I2C driver, RMT transmitter/receiver, ESP-DSP vector SIMD routines).
  - `src/platforms/targets/stm/`: Placeholder for target STM32 porting.

---

## 3. Centralized Task Management & Scheduling Rules

To eliminate scheduler fragmentation and priority inversion, all FreeRTOS task configurations are managed centrally in [`include/config.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/include/config.h).

### Structured Task Parameter Enums
Hardcoding magic numbers for priorities, stack sizes, or frequencies is strictly prohibited. Configurations are organized into 4 enum groups:

```cpp
enum TaskPriorities : uint8_t {
  TASK_PRIORITY_LOW      = 1, // Telemetry, Housekeeping, Logging
  TASK_PRIORITY_MED      = 2, // Barometer (50 Hz), Magnetometer (75 Hz) DAQ
  TASK_PRIORITY_HIGH     = 3, // Rate Controller, Motor Output
  TASK_PRIORITY_REALTIME = 4  // IMU DAQ Loop (500 Hz)
};

enum TaskFrequencies : uint16_t {
  TASK_ACCEL_GYRO_DAQ_FREQ_HZ  = 500,
  TASK_MAG_DAQ_FREQ_HZ         = 75,
  TASK_BARO_DAQ_FREQ_HZ        = 50
};
```

### Deterministic Periodic Delay Policy
All periodic sensor tasks and control loops MUST use `vTaskDelayUntil(&last_wake_time, period_ticks)` rather than standard `vTaskDelay()`. This ensures zero execution phase drift across iterations.

---

## 4. Middleware Isolation & Inter-Task IPC (`src/middleware/`)

### Centralized IPC Registration (`ipc.h` / `ipc.cpp`)
All shared inter-task communication handles—including FreeRTOS mutexes (`i2c0_mutex`, `i2c1_mutex`), queues, semaphores, and static ring buffers (`imu_ring_buffer`)—are declared in [`src/middleware/ipc.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/ipc.h) and allocated in [`src/middleware/ipc.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/ipc.cpp). System IPC handles are initialized globally during boot via `init_ipc()`.

### I2C Bus Mutex Synchronization Rule
Driver tasks sharing physical I2C buses (Bus 0 / Bus 1) MUST wrap all read/write transactions with `xSemaphoreTake()` and `xSemaphoreGive()` using centralized bus mutex handles:

```cpp
if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(10)) == pdTRUE){
  mpu.read_raw(&accel_raw, &gyro_raw);
  xSemaphoreGive(i2c0_mutex);
}
```

### ISR & Completion Callback Routing
Low-level hardware Interrupt Service Routines (ISRs) and asynchronous transaction completion callbacks MUST be defined inside Layer 3 (`src/middleware/`). High-level driver or application code must never contain raw `ISR_ATTR` functions directly.

---

## 5. Layer 1 SIMD HAL & Multi-Type Acceleration

### Layer 1 Hardware Purity & Zero-Fallback Rule
Target SIMD files ([`src/platforms/targets/espressif/simd.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/targets/espressif/simd.cpp)) serve strictly as physical hardware execution drivers:
- **No `#else` Fallbacks**: Layer 1 SIMD functions must NOT contain scalar `#else` fallback implementations.
- **Hardware-Only Scope**: Layer 1 `hal_simd` provides routines exclusively for types supported directly by hardware SIMD instructions (`float`, `int16_t`, `int8_t`).

### Layer 4 Matrix Architecture & Selection Logic (`matrix.h`)
Layer 4 ([`src/core/math/matrix.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/math/matrix.h)) owns compilation flag checking (`ENABLE_SIMD_ACCELERATION`) and type dispatching:
- **`Matrix<T, ROWS, COLS>`**: Unified template class for linear algebra calculations.
- **`Vec3<T>` Inheritance & Union**: Inherits `Matrix<T, 3, 1>` with an anonymous union (`union { T m[3]; struct { T x, y, z; }; }`), supporting both vector member access (`x, y, z`) and contiguous array buffer indexing (`m[i]`).
- **Preprocessor Block Ordering**:
  - Top Block (`#if (!ENABLE_SIMD_ACCELERATION)`): Portable C++ scalar loops.
  - Bottom Block (`#else`): Hardware SIMD dispatch via `if constexpr` for `float`, `int16_t`, `int8_t`. Types lacking target SIMD support (`int32_t`, `double`) gracefully execute scalar fallbacks inside Layer 4.

### ESP-DSP Hardware Primitive Integration
For ESP32-S3 targets, SIMD primitives map directly to Espressif ESP-DSP assembly routines:
- **32-Bit Float (`float`)**: `dsps_dotprod_f32`, `dsps_add_f32`, `dsps_sub_f32`, `dsps_mulc_f32`, `dspm_mult_f32`.
- **16-Bit Integer (`int16_t`)**: 128-bit packed SIMD via `dsps_dotprod_s16`, `dsps_add_s16`, `dsps_sub_s16`, `dsps_mulc_s16`.
- **8-Bit Integer (`int8_t`)**: 128-bit packed SIMD via `dsps_dp_s8`, `dsps_add_s8`, `dsps_sub_s8`.

---

## 6. Hardware Peripheral Integration

### RMT PPM RC Receiver Decoding (ESP32-S3)
Pulse Position Modulation (PPM) decoding for RC receivers is offloaded to the ESP32-S3 **RMT (Remote Control Transceiver)** hardware peripheral on RX channel 0. Measuring pulse edge timings in hardware eliminates CPU interrupt starvation and preserves execution cycles for the 500 Hz flight loop.

---

## 7. System Coding Style & Formatting Rules

To maintain high readability and clean git diffs across developers, all C++ code in `FC_v01` strictly adheres to the following formatting rules:

1. **Single-Line Control Flow Bracing**: Omit curly braces `{}` for single-line control flow bodies:
   ```cpp
   if(condition) return false;
   for(uint8_t i = 0; i < N; ++i) sum += data[i];
   ```
2. **Function Declaration Bracing**: No space between closing parenthesis and opening brace:
   ```cpp
   void calculate_pid(){
     // ...
   }
   ```
3. **Control Flow Keyword & Opening Brace Spacing**: No space between keyword and parenthesis, and no space around `else`:
   ```cpp
   if(condition){
     // ...
   }else{
     // ...
   }
   ```
4. **Namespace Indentation**: Indent all contents inside `namespace` blocks by 2 spaces (1 level):
   ```cpp
   namespace hal_simd {
     void add_f32(const float* a, const float* b, float* out, uint16_t len);
   }
   ```
5. **Absolute Prohibition of `auto`**: Variable type deduction (`auto`) is strictly forbidden across all layers. Every variable must explicitly state its concrete type.
6. **Explicit Physical SI Units**: Standard SI units must be annotated in variable declaration comments (e.g., `float accel; // m/s^2`, `float gyro; // rad/s`).

---

## 8. Zettelkasten Traceability Index

| Zettel ID | Note Title | Main Architecture Section |
|---|---|---|
| `[0000]` | Firmware 5-Layer Architectural Decision & Directory Layout | Section 2 (5-Layer Architecture) |
| `[0001]` | Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding | Section 3 (Task Config), Section 4 (ISRs), Section 6 (RMT PPM) |
| `[0002]` | System 4 Priority Levels, Enum Task Config, Target-Agnostic HAL, Explicit Typing, Deterministic Delays & Centralized IPC | Section 3 (Enums & Delays), Section 4 (IPC), Section 7 (Coding Style) |
| `[0003]` | Layer 1 SIMD HAL Architecture & Zero-Fallback Rule | Section 5 (SIMD Zero-Fallback & Layer Purity) |
| `[0004]` | Multi-Type SIMD Hardware Acceleration & ESP-DSP Integration | Section 5 (Multi-Type SIMD, Matrix, ESP-DSP) |
| `[0006]` | Centralized Task Parameter Enums, Mandatory TaskIDs & Direct Driver Pointer Argument Passing | Section 3 (Task Config & TaskIDs) |
| `[0007]` | SEGGER SystemView Trace Instrumentation, OpenOCD Dual-Core Setup & RTOS Loop Profiling Conventions | Section 3 (Timing & Tracing Guardrails) |
| `[0008]` | Layer 4 Sensor Preprocessing & Calibration Pipeline Architecture | Section 3 (Sensor Preprocessing & Core Tasks) |
| `[0009]` | Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision | Section 4 (BLE Telemetry Architecture) |
| `[0010]` | BLE Minimum Packet Frequency and Dual-Trigger Flush Architecture | Section 4 (Telemetry Timing & Flushes) |
| `[0011]` | Variable BLE Telemetry Packet Parsing and Zero-Handshake Design | Section 4 (FCDatMon Telemetry Protocol) |
| `[0012]` | WS2812 RGB LED External Hardware Isolation and Centralized Mutex Encapsulation Rule | Section 2 (Hardware Composition) & Section 4 (IPC Mutexes) |
| `[0013]` | Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage | Section 1 (HAL), Section 2 (Storage), Section 3 (Logging Task) |
| `[0015]` | BLE OTA Lifecycle Architecture, Resource Stewardship & Host Stack Decoupling | Section 3 (OTA Task Config), Section 4 (BLE Architecture) |
| `[0016]` | Layer 4 Core Directory Compaction, Submodule Consolidation & Task Renaming Specification | Section 2 (Layer 4 Architecture) & Section 3 (Task Organization) |
| `[0017]` | BMP180 Non-Blocking Two-Phase I2C DAQ Architecture, Pipelined Conversion & Vertical Velocity Fusion Specification | Section 3 (Baro DAQ Task) & Section 4 (IPC) |
| `[0018]` | MicroSD Flight Logging Architecture, POSIX Metadata Sync, User Button Shutdown & Binary Log Specification | Section 2 (Storage), Section 3 (Logging Task), Section 4 (Shutdown ISR) |
| `[0019]` | Universal Flight Data Analyzer Architecture, Binary Telemetry 4-Byte Struct Alignment & Offline Multi-Rate Signal Processing Pipeline | Section 2 (Blackbox Protocol & Tooling), Section 3 (Log Formats) |
| `[0020]` | Finite State Machine (FSM) Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture Specification | Section 2 (Layer 5 App Domain), Section 3 (Task Config & FSM), Section 4 (Preflight Checks & Safety Gating) |
| `[0021]` | BLE Telemetry Dispatcher Consolidation, Subscription-Aware Gating & FCDatMon Stride Specification | Section 2 (Layer 4 Telemetry), Section 3 (Task Config & BLE Dispatcher), Section 4 (GATT Streaming Protocol) |
| `[0022]` | Unified Pressure Ring Buffer, Filtered Sensor Variable Consolidation & Phantom Attitude Task Removal | Section 3 (Task Enums & Phantom Task Removal), Section 4 (IPC Ring Buffers & Baro Unification) |
| `[0023]` | FSM-Centric Sensor Calibration Orchestration and RGB LED Status Architecture Specification | Section 2 (Layer 5 App Domain), Section 3 (FSM), Section 4 (LED & Calib IPC) |
| `[0024]` | Internal Flash NVS Storage HAL, FlashStorageDriver & Diagonal Soft-Iron SIMD Calibration Persistence Specification | Section 1 (HAL Flash), Section 2 (FlashStorageDriver), Section 4 (IPC calib_data) |
| `[0025]` | 3D Rotated Ellipsoid Magnetometer Calibration (SIMD Adam), EKF Unit-Sphere Normalization, Task-Stack Memory Lifecycle & FSM LED Coordination | Section 3 (Calib Mag Task), Section 4 (IPC calib_data & mag), Section 5 (SIMD Math Engine) |
| `[0026]` | 6-Point Accelerometer Calibration via SIMD-Accelerated 6-Parameter Axis-Aligned Ellipsoid Fit, Autonomous Stationary Gating & FSM LED Coordination | Section 3 (Calib Accel Task), Section 4 (IPC calib_data & accel), Section 5 (SIMD Math Engine) |
| `[0027]` | Pre-Flight Sensor Data Stream Health Check, Multi-Rate Latency Verification & FSM Transition Gate Architecture | Section 2 (Layer 5 App Domain), Section 3 (Preflight Checks), Section 4 (IPC Stream Latency & Health) |
| `[0028]` | INA226 Power Module I2C1 DAQ, Shunt Resistor Calibration, Millis Scale Metrics & Battery Health Monitoring Architecture | Section 1 (I2C1 HAL), Section 2 (INA226 Driver), Section 3 (Power DAQ & Config), Section 4 (IPC & Telemetry), Section 5 (FSM Battery Gating) |
| `[0029]` | Unification of Inter-Task Ring Buffer Capacity Constant | Section 3 (OSAL Config), Section 4 (Layer 3 IPC Ring Buffers), Section 4 (SD Logger Snapshot Buffers) |
| `[0030]` | ESP32-S3 Hardware RMT PPM Receiver Decoding (Flysky FS-iA6B), Layer 1-4 Decoupled Architecture & BLE Real-Time Telemetry Pipeline | Section 1 (HAL Pulse RX & RMT Target), Section 2 (FS-iA6B Driver), Section 3 (Task Config & RC DAQ), Section 4 (IPC rc_data & BLE Telemetry), Section 5 (FSM Preflight Gating) |
| `[0031]` | Six-Gate Pre-Flight Safety Pipeline, Terminal Development Mode, Subsystem Task Encapsulation & Battery Enforcement Architecture | Section 2 (Layer 5 App Domain), Section 3 (Task Encapsulation & SSM Lifecycle), Section 4 (Preflight Gating, Failover & Battery Arming) |
| `[0032]` | RC Receiver Command MicroSD Blackbox Logging Architecture, 4-Byte Struct Alignment & Ring Buffer Snapshot Extraction | Section 1 (HAL Pulse RX), Section 2 (FS-iA6B Driver), Section 3 (SD Logger Task), Section 4 (IPC rc_data & Snapshot Buffers), Section 5 (Config & Packet Mask) |



