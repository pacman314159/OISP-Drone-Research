# Graph Report - FC_v01  (2026-10-28)

## Corpus Check
- 154 files · ~315,850 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 11 file(s) not represented in the graph (top: (none) 5, .lock 1, .csv 1)

## Summary
- 1915 nodes · 2387 edges · 714 communities (68 shown, 646 thin omitted)
- Extraction: 87% EXTRACTED · 13% INFERRED · 0% AMBIGUOUS · INFERRED: 312 edges (avg confidence: 0.87)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `47ab1101`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- ble_packet.h
- Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture
- Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing
- INA226
- BMP180
- ESP32_SPI
- fsm.cpp
- HMC5883L
- Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision
- System Life Cycle Document (Placeholder)
- sd_logger_task.cpp
- Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage
- HAL_I2C_ASYNC
- mpu6050.cpp
- Attitude Outer Loop Task (100Hz)
- MotorDriver
- MPU6050
- Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions
- Firmware Coding Conventions & Architecture Rules
- main.cpp
- PktImuFilt
- HAL_PWM
- arduino
- FC_v01 Firmware Architecture & Structure Specification
- ESP32_I2C_ASYNC
- Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout
- 3. Core Architectural Decisions
- accel_gyro_daq_task
- BLE Telemetry 500 Hz Non-Blocking Data Flow
- app_start
- PktImuRaw
- FlashRecordHeader
- ble_driver.cpp
- ESP32_PWM_LEDC
- Matrix
- 3. Core Architectural Decision: Primitive Array Interface with Direct Layer 1 SIMD
- PktAttEuler
- LogFileHeader
- HAL_I2C
- PktAttQuat
- PktMagRaw
- FC_v01 Architectural & Codebase Context Analysis Skill
- Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision
- ESP32_PulseRX
- Main Firmware Component Build Configuration
- AltitudeEKFEngine
- IndiControllerOutput
- FC_v01 CMake Project
- I2C1 Bus Mutex Guard
- AttitudeEKFEngine
- SDCardDriver
- Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent
- WS2812
- Vec3
- HAL_PulseRX
- ble_ota_uploader.py
- ESP32_I2C
- PktPower
- Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture
- LowPassFilter2ndOrder
- 0037-threeStateVerticalVelocityAltitudeEkfAndBaroAccelFusionArchitecture.md
- PktMagFilt
- Zettel 0044: System Physical Parameters, Mass Budget & Plant Inertia Estimation
- HAL_SPI
- RingBuffer
- sd_card_driver.cpp
- 3. Rationale for Excluding Calibration Tasks
- 5. RTOS Task Topology, Inter-Process Communication & Dependency Injection
- PktRc
- Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent
- 5-Layer Decoupled Firmware Architecture
- 2. Core Architectural Principle: In-Flight Tracing Scope
- PktBaroRaw

## God Nodes (most connected - your core abstractions)
1. `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` - 52 edges
2. `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision` - 39 edges
3. `Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision` - 35 edges
4. `Vec3` - 34 edges
5. `INA226` - 33 edges
6. `MPU6050` - 30 edges
7. `FC_v01 Firmware Architecture & Structure Specification` - 29 edges
8. `Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing` - 28 edges
9. `SDCardDriver` - 27 edges
10. `Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers` - 27 edges

## Surprising Connections (you probably didn't know these)
- `Rule 1: Dependency Inversion for Layer 2 BLE Driver Decoupling` --references--> `app_start()`  [INFERRED]
  documents/notes/0039-onDemandBleOtaInDevelopmentStateAndDependencyInversionArchitecture.md → src/app/app.cpp
- `4. Cross-References & Traceability` --references--> `motor_task()`  [INFERRED]
  documents/notes/0034-motorDriverLedcHalQuadxActuatorTaskArchitecture.md → src/core/actuator/motor_tasks.cpp
- `Rule 4: Two-Stage Verification in `motor_task` ("The Second Check")` --references--> `motor_task()`  [INFERRED]
  documents/notes/0035-emergencyMotorDisarmMediatorAndLawOfDemeterDecision.md → src/core/actuator/motor_tasks.cpp
- `2. Problem Statement & Motivation` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0045-rcReceiverCommandBlackboxSdCardLoggingArchitecture.md → src/core/blackbox/sd_logger_task.cpp
- `Rule 2: Layer 2 Motor Peripheral Driver (`MotorDriver`)` --references--> `MotorDriver`  [INFERRED]
  documents/notes/0034-motorDriverLedcHalQuadxActuatorTaskArchitecture.md → src/drivers/motor/motor_driver.h

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Cascaded Attitude Control Architecture** — documents_rtos_architecture_attitude_outer_loop, documents_rtos_architecture_attitude_inner_loop, documents_rtos_architecture_motor_mixer [EXTRACTED 0.95]
- **IMU Multi-Sensor Fusion Pipeline** — documents_rtos_architecture_accel_gyro_calib_filter, documents_rtos_architecture_mag_calib_filter, documents_rtos_architecture_ekf_roll_pitch, documents_rtos_architecture_ekf_yaw [EXTRACTED 0.95]
- **Sensor Data Acquisition and Ring Buffer Decoupling** — documents_rtos_architecture_accel_gyro_daq, documents_rtos_architecture_mag_daq, documents_rtos_architecture_baro_daq, documents_rtos_architecture_ring_buffer_newest_index_rule [EXTRACTED 0.95]

## Communities (714 total, 646 thin omitted)

### Community 2 - "ble_packet.h"
Cohesion: 0.07
Nodes (18): BleAttQuatPacket, q, timestamp, BleBaroPacket, pressure, timestamp, BlePowerPacket, current_ma (+10 more)

### Community 5 - "Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture"
Cohesion: 0.13
Nodes (17): Deterministic Bitmask Event-Driven State Transition Engine, Pre-Flight Sensor Health Verification and Stationary Detection Gating, Non-Blocking User Button ISR and Decoupled FreeRTOS Queue Event Flow, Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture, Centralized FSM Orchestration of Active Sensor Calibration Sequences, Thread-Safe RGB LED Command Queue and Color Encoding Palette, Zettel 0023: FSM-Centric Sensor Calibration Orchestration and RGB LED Status Architecture, Persistent Storage of Sensor Calibration Offsets and Matrices in NVS Flash (+9 more)

### Community 6 - "Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing"
Cohesion: 0.12
Nodes (16): Centralized Enum Task Parameter Declarations (include/config.h), Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing, Centralized UUID Service & Characteristic Series (include/config.h), Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision, Centralized Frequency Floor (BLE_TELEM_MIN_PACKET_FREQ_HZ = 10), Zettel 0010: BLE Minimum Packet Frequency and Dual-Trigger Transmission Protocol, Dynamic User-Configured Sample Stride Architecture in FCDatMon, Zettel 0011: Variable BLE Telemetry Packet Parsing and Zero-Handshake Design (+8 more)

### Community 7 - "INA226"
Cohesion: 0.09
Nodes (31): INA226, calibrate, _current_lsb_ma, _dev_addr, get_all_data, get_all_data_raw, INA226::INA226(), init (+23 more)

### Community 9 - "BMP180"
Cohesion: 0.07
Nodes (30): baro_daq_task(), BMP180, _b5, BMP180::BMP180(), calculate_b5, calculate_pressure, calculate_temperature, _calib (+22 more)

### Community 16 - "ESP32_SPI"
Cohesion: 0.16
Nodes (9): ESP32_SPI, ESP32_SPI::ESP32_SPI(), init, _initialized, _miso_pin, _mosi_pin, _sck_pin, transfer (+1 more)

### Community 17 - "fsm.cpp"
Cohesion: 0.07
Nodes (48): 2. Problem Statement & Motivation, 3. Core Architectural Decisions, Rule 1: Establishment of `src/core/safety/` in Layer 4, Rule 2: Universal `SafetyStatus` Evaluation Enum, Rule 3: Layer 5 FSM Preflight Coordination, 1. Metadata & Scope, 2. Context & Problem Statement, 3.2 Terminal Calibration State Enforcement (+40 more)

### Community 19 - "HMC5883L"
Cohesion: 0.10
Nodes (18): Real-Time Flight Loop Calibration (m_calib = W * (m_raw - v0)), mag_daq_task(), HMC5883L, _dev_addr, get_all_data, get_all_data_raw, HMC5883L::HMC5883L(), init (+10 more)

### Community 22 - "Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision"
Cohesion: 0.12
Nodes (15): Centralized Inter-Task Communication in src/middleware/ipc.h and ipc.cpp, Deterministic Periodic Task Timing via vTaskDelayUntil, Structured Enum-Based Task Parameters (include/config.h), Strict 4-Level RTOS Priority Architecture, Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision, Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule, ESP-DSP Multi-Type SIMD Integration, Layer 1 hal_simd Multi-Type Hardware SIMD Acceleration (float, int16_t, int8_t) (+7 more)

### Community 24 - "sd_logger_task.cpp"
Cohesion: 0.23
Nodes (6): append_packet(), ChannelTracker, has_logged, last_ts, log_dual_stream(), log_single_stream()

### Community 26 - "Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage"
Cohesion: 0.10
Nodes (15): Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (sd_logger_task), 4-Digit Incremental File Scheme (LOG_0001.BIN) & LogFileHeader, Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage, 512-Byte Self-Describing Sector-Aligned Binary Log Header (LogFileHeader), Hardware USER Button 3-Second Hold Graceful Shutdown Protocol, Zettel 0018: MicroSD Flight Logging Architecture, POSIX Metadata Sync, User Button Shutdown & Binary Log Specification, Multi-Rate Binary Telemetry Timestamp Alignment and Interpolation, Universal Flight Data Analyzer Offline Multi-Channel Processing Pipeline (+7 more)

### Community 27 - "HAL_I2C_ASYNC"
Cohesion: 0.11
Nodes (6): HAL_I2C_ASYNC, init, read_reg_async, read_reg_multi_async, write_reg_async, write_reg_multi_async

### Community 28 - "mpu6050.cpp"
Cohesion: 0.16
Nodes (9): enable_bypass, enable_sleep, MPU6050::get_all_data_async(), get_all_data_raw, init, MPU6050::MPU6050(), set_accel_range, set_clock_source (+1 more)

### Community 29 - "Attitude Outer Loop Task (100Hz)"
Cohesion: 0.17
Nodes (14): Accel Gyro Calib & Filter Task (250Hz), Accel Gyro DAQ Task (500Hz), Altitude & Vz Estimator Task (100Hz), Attitude Inner Loop Task (250Hz, 4 Pri), Attitude Outer Loop Task (100Hz), Barometer Calib & Filter Task, Barometer DAQ Task, EKF Roll & Pitch Task (100Hz) (+6 more)

### Community 30 - "MotorDriver"
Cohesion: 0.23
Nodes (13): motor_task(), MotorDriver, arm_all, _armed, _current_norm, disarm_all, get_motor_normalized, init (+5 more)

### Community 31 - "MPU6050"
Cohesion: 0.10
Nodes (13): MPU6050, _accel_raw_x, _accel_raw_y, _accel_raw_z, _accel_sens, _dev_addr, _gyro_raw_x, _gyro_raw_y (+5 more)

### Community 32 - "Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions"
Cohesion: 0.14
Nodes (11): Heinz Guderian (RTOS & DSP Lead), OpenOCD Dual-Core Trace Capture Syntax & Sizing Sweet Spot, Python SystemView Profiling & Visualization Suite (/analyzers), Standardized RTOS Timing Analysis Rules, Dual-Core Flight Loop Steady-State Benchmarks, Sub-Slice Fragmentation vs Full-Loop Boundary Timing Analysis, Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions, 1. Metadata & Scope (+3 more)

### Community 34 - "Firmware Coding Conventions & Architecture Rules"
Cohesion: 0.17
Nodes (12): Firmware Coding Conventions & Architecture Rules, Single-Line Control Flow & No-Space Bracing Style, Centralized Task Parameters Rule, Coding Conventions & Architecture Rules, Explicit Physical SI Units Rule, SystemView Tracing & RTOS Timing Conventions, FreeRTOS Task Scoping Architecture Rule, Max Ducson (Chief Mechatronics Engineer) (+4 more)

### Community 35 - "main.cpp"
Cohesion: 0.15
Nodes (8): Layer 3: Middleware & Centralized Inter-Task IPC Layer, Centralized Inter-Task IPC in Layer 3 (src/middleware/), 3. Dependency Mapping & Graphify Subgraph Analysis, Affected Translation Units & Architectural Constraints:, app_main(), init_ipc(), sysview_init_markers(), init

### Community 36 - "PktImuFilt"
Cohesion: 0.18
Nodes (10): PktBaroFilt, pressure, timestamp_us, type, PktImuFilt, accel, gyro, _pad (+2 more)

### Community 37 - "HAL_PWM"
Cohesion: 0.18
Nodes (8): HAL_PWM, get_freq_hz, get_resolution_bits, init_channel, init_timer, set_duty_ticks, set_pulse_us, stop_channel

### Community 39 - "FC_v01 Firmware Architecture & Structure Specification"
Cohesion: 0.15
Nodes (8): Deterministic Periodic Delay Policy via vTaskDelayUntil, ESP-DSP Hardware SIMD Primitive Integration, Layer 4 Matrix<T, ROWS, COLS> Template Architecture, ESP32-S3 RMT Hardware PPM RC Receiver Decoding, Layer 1 SIMD HAL Zero-Fallback Hardware Purity Rule, FC_v01 Firmware Architecture & Structure Specification, Centralized Task Parameter Enums in config.h, Firmware Structure Zettelkasten Traceability Index

### Community 40 - "ESP32_I2C_ASYNC"
Cohesion: 0.06
Nodes (34): FlashStorageDriver, calculate_crc32, erase_namespace, erase_record, init, _initialized, read_record, write_record (+26 more)

### Community 41 - "Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout"
Cohesion: 0.10
Nodes (15): Sarah Thueson (Embedded Software Architect), 5-Layer Decoupled Firmware Architecture Decision, FreeRTOS Task Scoping Restricted to Layer 4 (core/) or Layer 5 (app/), Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout, Zettel 0012: WS2812 External Hardware Treatment & Mutex Encapsulation Rule, Client-Side ATT MTU Adaptation & Error Unblocking, Zettel 0015: BLE OTA Lifecycle Architecture, Resource Stewardship & Host Stack Decoupling, 1. Metadata & Scope (+7 more)

### Community 43 - "3. Core Architectural Decisions"
Cohesion: 0.22
Nodes (9): 1. Metadata & Scope, 2. Problem Statement & Motivation, 3. Core Architectural Decisions, 4. Cross-References & Traceability, Rule 1: Layer 1 Platform HAL Abstraction (`HAL_PWM` & `ESP32_PWM_LEDC`), Rule 2: Layer 2 Motor Peripheral Driver (`MotorDriver`), Rule 3: Layer 3 Inter-Task IPC (`IndiControllerOutput`), Rule 4: Layer 4 Actuator Domain (`src/core/actuator/motor_tasks.cpp/.h`) (+1 more)

### Community 44 - "accel_gyro_daq_task"
Cohesion: 0.33
Nodes (4): 500 Hz Core IMU Loop Frequency Rule, Mandatory TaskIDs Enum Enumeration, accel_gyro_daq_task(), get_all_data

### Community 45 - "BLE Telemetry 500 Hz Non-Blocking Data Flow"
Cohesion: 0.33
Nodes (6): BLE Telemetry 500 Hz Non-Blocking Data Flow, Custom Dual OTA Partition Scheme (8MB Layout), Project: FC_v01 Firmware & 5-Layer RTOS Architecture, 5-Layer Decoupled RTOS Architecture Project Scope, GyroTelemetrySample IPC Data Contract, Centralized Interface Contracts (config.h, ipc.h, ble_driver.h)

### Community 46 - "app_start"
Cohesion: 0.25
Nodes (9): 2. Problem Statement & Architectural Tension, 3. Core Architectural Decisions, Rule 1: Application Layer Exclusive Arming Authority, Rule 2: Layer 2 `MotorDriver` Internal Defense-in-Depth, Rule 3: Dual-Channel Mediator Architecture (Command Queue + Out-of-Band Notification), Rule 4: Two-Stage Verification in `motor_task` ("The Second Check"), Rule 5: Actuator Task Priority Elevation to Realtime (`4`), app_start() (+1 more)

### Community 48 - "PktImuRaw"
Cohesion: 0.33
Nodes (6): PktImuRaw, accel, gyro, _pad, timestamp_us, type

### Community 49 - "FlashRecordHeader"
Cohesion: 0.40
Nodes (5): FlashRecordHeader, crc32, magic, payload_len, schema_ver

### Community 50 - "ble_driver.cpp"
Cohesion: 0.05
Nodes (38): Driver Isolation & Explicit Notification APIs (send_ble_*_notification), Layer 4 Task Identity Preservation (TASK_BLE_OTA_ID), 2. Problem Statement & Context, 3. Core Architectural Decisions, Rule 1: Dependency Inversion for Layer 2 BLE Driver Decoupling, Rule 2: On-Demand Task & Queue Spawning (`spawn_ble_ota_task`), Rule 3: Strict In-Flight Safety Lockout, Rule 4: Protocol Extension for Remote Reboot (`OTA_CMD_REBOOT`) (+30 more)

### Community 51 - "ESP32_PWM_LEDC"
Cohesion: 0.18
Nodes (10): ESP32_PWM_LEDC, _channel_initialized, _freq_hz, init_channel, init_timer, _resolution_bits, set_duty_ticks, set_pulse_us (+2 more)

### Community 53 - "Matrix"
Cohesion: 0.26
Nodes (8): dot(), hadamard(), inverse3x3(), Matrix, norm(), norm_sq(), normalize(), operator+()

### Community 55 - "3. Core Architectural Decision: Primitive Array Interface with Direct Layer 1 SIMD"
Cohesion: 0.25
Nodes (7): 1. Metadata & Scope, 2. Problem Statement: Upward Dependency Inversion (Layer 2 $\rightarrow$ Layer 4), 3.1 Driver Primitive Buffer Interface, 3.2 In-Place Layer 1 Hardware SIMD Scaling, 3.3 Zero-Copy Layer 4 Integration, 3. Core Architectural Decision: Primitive Array Interface with Direct Layer 1 SIMD, Zettel 0043: Layer 2 Driver Decoupling from Layer 4 Math and Direct Layer 1 SIMD Scaling Architecture

### Community 57 - "PktAttEuler"
Cohesion: 0.40
Nodes (5): PktAttEuler, euler, _pad, timestamp_us, type

### Community 58 - "LogFileHeader"
Cohesion: 0.18
Nodes (11): LogFileHeader, board_name, boot_timestamp_us, circular_head_sector, limit_policy, magic, max_file_size_bytes, registered_packets_mask (+3 more)

### Community 59 - "HAL_I2C"
Cohesion: 0.29
Nodes (6): HAL_I2C, probe, read_reg, read_reg_multi, write_reg, write_reg_multi

### Community 60 - "PktAttQuat"
Cohesion: 0.40
Nodes (5): PktAttQuat, _pad, q, timestamp_us, type

### Community 61 - "PktMagRaw"
Cohesion: 0.40
Nodes (5): PktMagRaw, mag, _pad, timestamp_us, type

### Community 74 - "FC_v01 Architectural & Codebase Context Analysis Skill"
Cohesion: 0.25
Nodes (7): Graphify Knowledge Graph Rule, Graphify Query Before Grep Rule, FC_v01 Architectural & Codebase Context Analysis Skill, Codebase Graph Traversal Workflow, Zettelkasten Note Vault & ADR Scan Workflow, Workflow: analyze-context, analyze-context Execution Pipeline

### Community 79 - "Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision"
Cohesion: 0.15
Nodes (9): Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision, Strict Sequential 6-Gate Pre-Flight Safety Evaluation Pipeline, Zettel 0031: Six-Gate Pre-Flight Safety Pipeline, Terminal Development Mode, Subsystem Task Encapsulation & Battery Enforcement Architecture, 1. Metadata & Scope, 4. Cross-References & Traceability, Zettel 0035: Emergency Motor Disarm, Mediator Orchestration & Law of Demeter Architectural Decision, 1. Metadata & Scope, 4. Cross-References & Traceability (+1 more)

### Community 84 - "ESP32_PulseRX"
Cohesion: 0.05
Nodes (29): PulseSymbol, duration0_ns, duration1_ns, level0, level1, ESP32_PulseRX, init, initialized (+21 more)

### Community 110 - "AltitudeEKFEngine"
Cohesion: 0.18
Nodes (10): ekf_altitude_task(), AltitudeEKFEngine, _az_inertial, _p0_pa, _P_cov, _Q_cov, _R_baro, set_reference_pressure (+2 more)

### Community 129 - "IndiControllerOutput"
Cohesion: 0.10
Nodes (21): 3. Comprehensive OSAL & IPC Entity Audit, 4. Architectural Decisions & Execution Specification, Decision 1: Relocation of `led_task` to Unified `interface_tasks`, Decision 2: Executed Layer 3 IPC Refactoring & Domain Decoupling, PowerMeasurement, current_ma, energy_mah, power_mw (+13 more)

### Community 159 - "AttitudeEKFEngine"
Cohesion: 0.12
Nodes (21): ekf_roll_pitch_task(), ekf_yaw_task(), predict, reset, AttitudeEKFEngine, AttitudeEKFEngine::AttitudeEKFEngine(), _earth_bx, _earth_bz (+13 more)

### Community 161 - "SDCardDriver"
Cohesion: 0.16
Nodes (17): sd_logger_task(), SDCardDriver, _bytes_written, _cd_pin, _cs_pin, _file, find_highest_log_id, flush (+9 more)

### Community 172 - "Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent"
Cohesion: 0.14
Nodes (15): Empirical Flight Data Calibration Results & Universal Flight Analyzer Verification, ESP32-S3 128-Bit PIE SIMD Vector Dot Product Acceleration, Multivariate Analytical Gradient Derivation for Ellipsoid Optimization, Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent, Real-Time Flight Loop Matrix-Vector Ellipsoid Correction, 6-Parameter Symmetric Soft-Iron Matrix Formulation, 3D Rotated Ellipsoid to Unit-Sphere Mapping Formulation, Time-Sliced SIMD Adam Gradient Descent Optimizer for Ellipsoid Fitting (+7 more)

### Community 178 - "WS2812"
Cohesion: 0.12
Nodes (23): status_led_task(), Color, b, g, r, get_palette_color(), WS2812, brightness (+15 more)

### Community 205 - "Vec3"
Cohesion: 0.23
Nodes (13): lpf_apply(), Vec3, calib_apply_accel(), calib_apply_gyro(), calib_apply_mag(), calib_init(), calib_load_from_flash(), calib_reset_defaults() (+5 more)

### Community 226 - "HAL_PulseRX"
Cohesion: 0.09
Nodes (22): 1. Metadata & Scope, 2. Problem Statement: Protocol Logic Bleed into Layer 1 HAL, 3. Proposed Future Decoupling Solution, Architectural Deficiencies, Zettel 0047: Layer 1 Hardware RMT PulseRX PPM Bleed & Layer 2 Protocol Decoupling Analysis, channels, FS_iA6B, FS_iA6B::FS_iA6B() (+14 more)

### Community 261 - "ble_ota_uploader.py"
Cohesion: 0.13
Nodes (4): BleOtaUploader, find_firmware_binary(), main(), run_reboot()

### Community 536 - "ESP32_I2C"
Cohesion: 0.18
Nodes (10): ESP32_I2C, ESP32_I2C::ESP32_I2C(), init, probe, read_reg, read_reg_multi, _scl_pin, _sda_pin (+2 more)

### Community 604 - "PktPower"
Cohesion: 0.25
Nodes (8): PktPower, current_ma, energy_mah, _pad, power_mw, timestamp_us, type, voltage_mv

### Community 645 - "Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture"
Cohesion: 0.10
Nodes (19): BLE Characteristic UUID Isolation Reference, Dual Execution Engine (SIMD vs Scalar LPF), IPC Clean Variable Naming & Ring Buffer Consolidation, Layer 4 DSP Filter & Calibration Scoping Rule, Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture, Interleaved Low-Frequency Temperature Refresh at 1Hz, Non-Blocking Two-Phase I2C Transaction Model for Barometer, Barometric Vertical Velocity and Acceleration Drift Compensation (+11 more)

### Community 656 - "LowPassFilter2ndOrder"
Cohesion: 0.10
Nodes (18): Tustin Bilinear 1st-Order Low-Pass Filter State (LowPassFilter1stOrder), ButterworthFilter, LowPassFilter1stOrder, a, b, initialized, x_prev, y_prev (+10 more)

### Community 692 - "0037-threeStateVerticalVelocityAltitudeEkfAndBaroAccelFusionArchitecture.md"
Cohesion: 0.25
Nodes (7): 1. Metadata & Scope, 2. Problem Statement & Motivation, 3. Mathematical & Algorithmic Formulation, 4. Empirical Noise Covariance Grounding (`LOG_0032.BIN`), A. Coordinate Frame & Kinematic Acceleration (Z-Up), B. 3-State Extended Kalman Filter Equations, Zettel 0037: 3-State Vertical Velocity & Altitude Extended Kalman Filter (AltitudeEKFEngine), Dependency Injection & Layer 4 RTOS Task Architecture Specification

### Community 693 - "PktMagFilt"
Cohesion: 0.40
Nodes (5): PktMagFilt, mag, _pad, timestamp_us, type

### Community 694 - "Zettel 0044: System Physical Parameters, Mass Budget & Plant Inertia Estimation"
Cohesion: 0.33
Nodes (6): 1. Metadata & Scope, 2. Mass Breakdown & Measured Weight Budget, 3. Geometric & Frame Constants, 4. Derived Plant Inertia Estimation, 5. Cross-References & Traceability, Zettel 0044: System Physical Parameters, Mass Budget & Plant Inertia Estimation

### Community 695 - "HAL_SPI"
Cohesion: 0.15
Nodes (13): Unified HAL_SPI Abstraction & Shared Bus Decoupling (Layer 1), 1. Metadata & Scope, 2. Problem Statement: Interface Segregation Violation in `HAL_SPI`, 3.1 Pure Layer 1 Bus Interface (`HAL_SPI`), 3.2 High-Level Storage Encapsulation in `SDCardDriver` (Layer 2), 3. Core Architectural Decision: Clean Responsibility Split (Solution A), 4. Architectural Summary & Benefits, Zettel 0048: HAL SPI Interface Segregation & SD Card Driver Filesystem Decoupling (+5 more)

### Community 696 - "RingBuffer"
Cohesion: 0.23
Nodes (3): RingBuffer, _buffer, _head

### Community 697 - "sd_card_driver.cpp"
Cohesion: 0.20
Nodes (4): close, get_free_bytes, SDCardDriver::SDCardDriver(), unmount

### Community 698 - "3. Rationale for Excluding Calibration Tasks"
Cohesion: 0.40
Nodes (10): 3.1 Contamination of Preprocessing & Estimator State, 3.1 Severe Trace Metric Distortion, 3.2 JTAG Apptrace Buffer Saturation, 3.3 Separation of Concerns (Bench Setup vs. In-Flight Execution), 3. Rationale for Excluding Calibration Tasks, calib_save_to_nvs(), task_calib_accel(), task_calib_baro() (+2 more)

### Community 699 - "5. RTOS Task Topology, Inter-Process Communication & Dependency Injection"
Cohesion: 0.20
Nodes (9): Dedicated Task Mapping in src/core/telemetry/ble_telem_tasks.cpp, 5. RTOS Task Topology, Inter-Process Communication & Dependency Injection, 6. Verification & Traceability, PktAlt, _pad, state, timestamp_us, type (+1 more)

### Community 700 - "PktRc"
Cohesion: 0.22
Nodes (9): 3. Core Architectural Decisions, 4. Architectural Summary, Rule 1: Binary Packet Specification (`PktRc`) & 4-Byte Struct Alignment, Rule 2: Centralized Packet Type & Bitmask Registration, Rule 3: Lock-Free Snapshot Extraction & Timestamp Tracking, PktRc, _pad, timestamp_us (+1 more)

### Community 701 - "Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent"
Cohesion: 0.29
Nodes (7): Adam Optimizer Monotonic Convergence & 3-Plane Projection Verification, Quadric Mean Squared Error Cost Function e(v0, W), Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent, ESP32-S3 128-Bit SIMD Inner-Loop Acceleration (Vec4f Alignment), Multivariate Matrix Calculus Gradient Derivations for v0 and W, Hard-Iron Bias and Soft-Iron Distortion Problem Formulation, 6-Parameter Symmetric Soft-Iron Matrix Formulation (9-Parameter General Ellipsoid)

### Community 702 - "5-Layer Decoupled Firmware Architecture"
Cohesion: 0.33
Nodes (5): 5-Layer Decoupled Firmware Architecture, Layer 1: Platform HAL & Hardware Target Drivers, Layer 2: Sensor & Peripheral Drivers Layer, Layer 4: Flight Core & Algorithms Layer, Layer 5: Application Layer

### Community 703 - "2. Core Architectural Principle: In-Flight Tracing Scope"
Cohesion: 0.50
Nodes (4): 1. Metadata & Scope, 2. Core Architectural Principle: In-Flight Tracing Scope, 4. Architectural Rules & Implementation Policy, Zettel 0041: SEGGER SystemView Tracing Scope & Exclusion of Maintenance and Calibration Tasks

### Community 704 - "PktBaroRaw"
Cohesion: 0.50
Nodes (4): PktBaroRaw, pressure, timestamp_us, type

## Knowledge Gaps
- **397 isolated node(s):** `type`, `timestamp_us`, `_pad`, `gyro`, `accel` (+392 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1241 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **646 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` connect `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` to `Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions`, `app.cpp`, `HAL_PulseRX`, `main.cpp`, `Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture`, `Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing`, `FC_v01 Firmware Architecture & Structure Specification`, `Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture`, `HAL_SPI`, `FC_v01 Architectural & Codebase Context Analysis Skill`, `BLE Telemetry 500 Hz Non-Blocking Data Flow`, `Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision`, `0037-threeStateVerticalVelocityAltitudeEkfAndBaroAccelFusionArchitecture.md`, `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision`, `3. Core Architectural Decision: Primitive Array Interface with Direct Layer 1 SIMD`, `Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage`, `Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent`?**
  _High betweenness centrality (0.059) - this node is a cross-community bridge._
- **Why does `Vec3` connect `Vec3` to `IndiControllerOutput`, `ble_packet.h`, `PktImuFilt`, `FC_v01 Firmware Architecture & Structure Specification`, `Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent`, `AltitudeEKFEngine`, `PktImuRaw`, `LowPassFilter2ndOrder`, `PktMagFilt`, `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision`, `Matrix`, `PktAttEuler`, `5. RTOS Task Topology, Inter-Process Communication & Dependency Injection`, `PktMagRaw`, `AttitudeEKFEngine`?**
  _High betweenness centrality (0.038) - this node is a cross-community bridge._
- **Why does `HAL_PWM` connect `HAL_PWM` to `app.cpp`, `3. Core Architectural Decisions`, `config.h`, `ESP32_PWM_LEDC`, `MotorDriver`?**
  _High betweenness centrality (0.026) - this node is a cross-community bridge._
- **Are the 2 inferred relationships involving `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` (e.g. with `Zettelkasten Decision Notes Rules` and `control_tasks.cpp`) actually correct?**
  _`Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` has 2 INFERRED edges - model-reasoned connections that need verification._
- **What connects `type`, `timestamp_us`, `_pad` to the rest of the system?**
  _397 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `ble_packet.h` be split into smaller, more focused modules?**
  _Cohesion score 0.07407407407407407 - nodes in this community are weakly interconnected._
- **Should `Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture` be split into smaller, more focused modules?**
  _Cohesion score 0.13438735177865613 - nodes in this community are weakly interconnected._