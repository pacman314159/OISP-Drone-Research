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
  - "[[0005-ws2812bDriverAndFutureStatusPresetsDecision]]"
---

# FC_v01 Firmware Architecture & Structure Specification

## 1. Executive Summary & Architectural Philosophy

The `FC_v01` flight controller firmware is designed for hard real-time, deterministic quadrotor flight control. Built on FreeRTOS and optimized for target microcontrollers (such as the Espressif ESP32-S3), the system guarantees predictable loop execution, strict OSAL/middleware isolation, target-agnostic physical hardware abstraction, and hardware-accelerated SIMD math computation.

### Core Architectural Principles
1. **Strict 500 Hz IMU Flight Loop Rate**: Primary sensor data acquisition (DAQ) and cascaded PID control loops operate at deterministic intervals (2.0 ms execution budget).
2. **Zero Dynamic Memory Allocation During Flight**: All RTOS tasks, ring buffers, queues, semaphores, and math structures are allocated statically during initialization. Zero `malloc()`, `free()`, `new`, or `delete` calls are permitted inside the flight execution loop.
3. **Strict Downward Dependency Hierarchy**: Higher application and flight core layers may consume lower layer abstractions, but lower layers (drivers, middleware, HAL) must never import or depend on higher layers.
4. **Target Hardware Isolation**: Driver, middleware, and core math modules across Layers 2 through 5 are entirely target-agnostic and consume standard HAL interfaces (`HAL_I2C`, `HAL_RGB_LED`, `hal_simd`). Target-specific SDK headers (`<Wire.h>`, `esp_err.h`, `stm32f4xx_hal.h`) are restricted exclusively to Layer 1 target modules (`src/platforms/targets/`).

---

## 2. 5-Layer Firmware Architecture & Directory Layout

The codebase is organized into **5 decoupled structural layers**:

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
  - `src/core/daq/`: High-frequency sensor Data Acquisition (DAQ) task runners ([`daq_tasks.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/daq/daq_tasks.cpp), [`daq_tasks.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/daq/daq_tasks.h)).
  - `src/core/control/`: Cascaded PID rate and attitude angle controllers, motor mixer.
  - `src/core/estimators/`: Orientation (AHRS) and altitude estimation (Complementary & Kalman filters).
  - `src/core/math/`: Linear algebra matrix engine ([`matrix.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/math/matrix.h)), unit conversions ([`conversions.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/core/math/conversions.h)), quaternions, and fast vector math.
  - `src/core/telemetry/`: High-level telemetry packaging and protocol formatting.

#### Layer 3: Middleware & Centralized Inter-Task IPC Layer
- **Directory**: [`src/middleware/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware)
  - Centralizes all inter-task communication primitives, hardware ISR routing, and lock-free thread-safe queues ([`ipc.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/ipc.h), [`ipc.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/ipc.cpp)).
  - Contains thread-safe lock-free static ring buffers ([`ring_buffer.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/ring_buffer.h)) and critical section macros ([`critical_section.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/middleware/critical_section.h)).

#### Layer 2: Sensor & Peripheral Drivers Layer
- **Directory**: [`src/drivers/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers)
  - Sensor-specific drivers: IMU ([`mpu6050`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/imu/mpu6050.h), [`hmc5883l`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/imu/hmc5883l.h)), Barometer ([`bmp180`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/baro/bmp180.h), `bmp280`, `ms5611`), RC RX ([`rc_rx`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/rc_rx)), LED ([`ws2812b`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/led/ws2812b.h)).
  - Hardware peripheral drivers are strictly stateless/class-based drivers; FreeRTOS task creation is forbidden inside Layer 2.

#### Layer 1: Platform HAL & Hardware Target Drivers Layer
- **Directory**: [`src/platforms/`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms)
  - `src/platforms/hal/`: Target-agnostic C++ pure abstract interfaces ([`hal_i2c.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/hal/hal_i2c.h), [`hal_i2c_async.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/hal/hal_i2c_async.h), [`hal_rgb_led.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/hal/hal_rgb_led.h), [`hal_simd.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/hal/hal_simd.h)).
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
  TASK_PRIORITY_HIGH     = 3, // Attitude & Rate Controller, Motor Output
  TASK_PRIORITY_REALTIME = 4  // IMU DAQ Loop (500 Hz)
};

enum TaskFrequencies : uint16_t {
  TASK_IMU_DAQ_FREQ_HZ  = 500,
  TASK_ATTITUDE_FREQ_HZ = 250,
  TASK_MAG_DAQ_FREQ_HZ   = 75,
  TASK_BARO_DAQ_FREQ_HZ  = 50
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

### WS2812B RGB LED Driver Architecture
- **Layer 1 HAL Driver**: Hardware-pure RMT driver ([`rgb_led_rmt.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/platforms/targets/espressif/rgb_led_rmt.cpp)) bound to GPIO 47 (`RGB_LED_PIN`).
- **Layer 2 Minimal Peripheral Driver**: Light API wrapper ([`ws2812b.h`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/led/ws2812b.h), [`ws2812b.cpp`](file:///d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01/src/drivers/led/ws2812b.cpp)) exposing direct functions (`init()`, `set_color()`, `set_hex()`, `off()`). No dedicated RTOS task is spawned for the LED.
- **Future Status Presets Roadmap**: System status color schemes (Booting = Blue, Disarmed = Green, Armed = Red, Error = Magenta) will be integrated directly into the central FSM (`src/app/fsm.cpp`).

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
| `[0005]` | WS2812B Hardware-Pure Layer 1 HAL, Layer 2 Minimal Driver & Future Status Presets Roadmap | Section 6 (WS2812B RGB LED Driver & Roadmap) |
