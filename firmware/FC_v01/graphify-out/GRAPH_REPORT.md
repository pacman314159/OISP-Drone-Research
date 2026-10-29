# Graph Report - FC_v01  (2026-10-28)

## Corpus Check
- 155 files · ~316,309 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 11 file(s) not represented in the graph (top: (none) 5, .lock 1, .csv 1)

## Summary
- 1923 nodes · 2409 edges · 713 communities (66 shown, 647 thin omitted)
- Extraction: 87% EXTRACTED · 13% INFERRED · 0% AMBIGUOUS · INFERRED: 317 edges (avg confidence: 0.87)
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
- HAL_SPI
- fsm.cpp
- HMC5883L
- Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision
- System Life Cycle Document (Placeholder)
- log_dual_stream
- Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage
- mpu6050.cpp
- Attitude Outer Loop Task (100Hz)
- MotorDriver
- MPU6050
- Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions
- Firmware Coding Conventions & Architecture Rules
- 5-Layer Decoupled Firmware Architecture
- PktImuFilt
- HAL_PWM
- config.h
- FC_v01 Firmware Architecture & Structure Specification
- HAL_Flash
- Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision
- 3. Core Architectural Decisions
- accel_gyro_daq_task
- BLE Telemetry 500 Hz Non-Blocking Data Flow
- app_start
- PktImuRaw
- FlashRecordHeader
- ble_driver.cpp
- ESP32_PWM_LEDC
- Matrix
- 0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling.md
- PktAttEuler
- LogFileHeader
- HAL_I2C
- ESP32_I2C_ASYNC
- PktMagRaw
- FC_v01 Architectural & Codebase Context Analysis Skill
- Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout
- ESP32_PulseRX
- Main Firmware Component Build Configuration
- BleTelemetryChannel
- ipc.cpp
- FC_v01 CMake Project
- I2C1 Bus Mutex Guard
- AltitudeEKFEngine
- SDCardDriver
- Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent
- WS2812
- Vec3
- FS_iA6B
- ble_ota_uploader.py
- ESP32_I2C
- HAL_PulseRX
- PktPower
- Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers
- LowPassFilter2ndOrder
- 0037-threeStateVerticalVelocityAltitudeEkfAndBaroAccelFusionArchitecture.md
- PktMagFilt
- Zettel 0044: System Physical Parameters, Mass Budget & Plant Inertia Estimation
- Zettel 0027: Pre-Flight Sensor Data Stream Health Check, Multi-Rate Latency Verification & FSM Transition Gate Architecture
- RingBuffer
- HAL_I2C_ASYNC
- LowPassFilter1stOrder
- BleVec3Packet
- PktRc
- PktBaroFilt
- ble_driver_init
- PktBaroRaw

## God Nodes (most connected - your core abstractions)
1. `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` - 53 edges
2. `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision` - 39 edges
3. `Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision` - 35 edges
4. `Vec3` - 34 edges
5. `INA226` - 33 edges
6. `MPU6050` - 31 edges
7. `FC_v01 Firmware Architecture & Structure Specification` - 29 edges
8. `SDCardDriver` - 28 edges
9. `Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing` - 28 edges
10. `Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers` - 27 edges

## Surprising Connections (you probably didn't know these)
- `Rule 1: Dependency Inversion for Layer 2 BLE Driver Decoupling` --references--> `app_start()`  [INFERRED]
  documents/notes/0039-onDemandBleOtaInDevelopmentStateAndDependencyInversionArchitecture.md → src/app/app.cpp
- `2. Context & Problem Statement` --references--> `handle_state_calib()`  [INFERRED]
  documents/notes/0040-fsmCalibrationTerminalStateAndRebootBoundaryDecision.md → src/app/fsm.cpp
- `4. Cross-References & Traceability` --references--> `motor_task()`  [INFERRED]
  documents/notes/0034-motorDriverLedcHalQuadxActuatorTaskArchitecture.md → src/core/actuator/motor_tasks.cpp
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

## Communities (713 total, 647 thin omitted)

### Community 2 - "ble_packet.h"
Cohesion: 0.09
Nodes (15): BleAttQuatPacket, q, timestamp, BleBaroPacket, pressure, timestamp, BlePowerPacket, current_ma (+7 more)

### Community 5 - "Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture"
Cohesion: 0.11
Nodes (22): Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule, ESP-DSP Multi-Type SIMD Integration, Layer 1 hal_simd Multi-Type Hardware SIMD Acceleration (float, int16_t, int8_t), Unified Matrix<T, ROWS, COLS> Template Architecture, Zettel 0004: Multi-Type SIMD Acceleration & ESP-DSP Build Integration, Deterministic Bitmask Event-Driven State Transition Engine, Pre-Flight Sensor Health Verification and Stationary Detection Gating, Non-Blocking User Button ISR and Decoupled FreeRTOS Queue Event Flow (+14 more)

### Community 6 - "Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing"
Cohesion: 0.11
Nodes (20): Centralized Enum Task Parameter Declarations (include/config.h), Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing, Centralized UUID Service & Characteristic Series (include/config.h), Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision, Centralized Frequency Floor (BLE_TELEM_MIN_PACKET_FREQ_HZ = 10), Zettel 0010: BLE Minimum Packet Frequency and Dual-Trigger Transmission Protocol, Dynamic User-Configured Sample Stride Architecture in FCDatMon, Zettel 0011: Variable BLE Telemetry Packet Parsing and Zero-Handshake Design (+12 more)

### Community 7 - "INA226"
Cohesion: 0.09
Nodes (32): power_daq_task(), INA226, calibrate, _current_lsb_ma, _dev_addr, get_all_data, get_all_data_raw, INA226::INA226() (+24 more)

### Community 9 - "BMP180"
Cohesion: 0.07
Nodes (30): baro_daq_task(), BMP180, _b5, BMP180::BMP180(), calculate_b5, calculate_pressure, calculate_temperature, _calib (+22 more)

### Community 16 - "HAL_SPI"
Cohesion: 0.05
Nodes (34): Unified HAL_SPI Abstraction & Shared Bus Decoupling (Layer 1), 1. Metadata & Scope, 2. Problem Statement: Interface Segregation Violation in `HAL_SPI`, 3.1 Pure Layer 1 Bus Interface (`HAL_SPI`), 3.2 High-Level Storage Encapsulation in `SDCardDriver` (Layer 2), 3. Core Architectural Decision: Clean Responsibility Split (Solution A), 4. Architectural Summary & Benefits, Zettel 0048: HAL SPI Interface Segregation & SD Card Driver Filesystem Decoupling (+26 more)

### Community 17 - "fsm.cpp"
Cohesion: 0.08
Nodes (39): Dedicated Task Mapping in src/core/telemetry/ble_telem_tasks.cpp, 2. Problem Statement & Motivation, 3. Core Architectural Decisions, Rule 1: Establishment of `src/core/safety/` in Layer 4, Rule 2: Universal `SafetyStatus` Evaluation Enum, Rule 3: Layer 5 FSM Preflight Coordination, 4. Architectural Rules & Implementation Policy, check_match_policy() (+31 more)

### Community 19 - "HMC5883L"
Cohesion: 0.10
Nodes (18): Real-Time Flight Loop Calibration (m_calib = W * (m_raw - v0)), mag_daq_task(), HMC5883L, _dev_addr, get_all_data, get_all_data_raw, HMC5883L::HMC5883L(), init (+10 more)

### Community 22 - "Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision"
Cohesion: 0.14
Nodes (12): Adam Optimizer Monotonic Convergence & 3-Plane Projection Verification, Quadric Mean Squared Error Cost Function e(v0, W), Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent, ESP32-S3 128-Bit SIMD Inner-Loop Acceleration (Vec4f Alignment), Multivariate Matrix Calculus Gradient Derivations for v0 and W, Hard-Iron Bias and Soft-Iron Distortion Problem Formulation, 6-Parameter Symmetric Soft-Iron Matrix Formulation (9-Parameter General Ellipsoid), Centralized Inter-Task Communication in src/middleware/ipc.h and ipc.cpp (+4 more)

### Community 24 - "log_dual_stream"
Cohesion: 0.22
Nodes (7): append_packet(), ChannelTracker, has_logged, last_ts, log_dual_stream(), log_single_stream(), write_sector

### Community 26 - "Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage"
Cohesion: 0.13
Nodes (12): Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (sd_logger_task), 4-Digit Incremental File Scheme (LOG_0001.BIN) & LogFileHeader, Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage, 512-Byte Self-Describing Sector-Aligned Binary Log Header (LogFileHeader), Hardware USER Button 3-Second Hold Graceful Shutdown Protocol, Zettel 0018: MicroSD Flight Logging Architecture, POSIX Metadata Sync, User Button Shutdown & Binary Log Specification, Multi-Rate Binary Telemetry Timestamp Alignment and Interpolation, Universal Flight Data Analyzer Offline Multi-Channel Processing Pipeline (+4 more)

### Community 28 - "mpu6050.cpp"
Cohesion: 0.16
Nodes (9): enable_bypass, enable_sleep, MPU6050::get_all_data_async(), get_all_data_raw, init, MPU6050::MPU6050(), set_accel_range, set_clock_source (+1 more)

### Community 29 - "Attitude Outer Loop Task (100Hz)"
Cohesion: 0.17
Nodes (14): Accel Gyro Calib & Filter Task (250Hz), Accel Gyro DAQ Task (500Hz), Altitude & Vz Estimator Task (100Hz), Attitude Inner Loop Task (250Hz, 4 Pri), Attitude Outer Loop Task (100Hz), Barometer Calib & Filter Task, Barometer DAQ Task, EKF Roll & Pitch Task (100Hz) (+6 more)

### Community 30 - "MotorDriver"
Cohesion: 0.23
Nodes (10): MotorDriver, _armed, _current_norm, disarm_all, get_motor_normalized, init, _initialized, MotorDriver::MotorDriver() (+2 more)

### Community 31 - "MPU6050"
Cohesion: 0.10
Nodes (13): MPU6050, _accel_raw_x, _accel_raw_y, _accel_raw_z, _accel_sens, _dev_addr, _gyro_raw_x, _gyro_raw_y (+5 more)

### Community 32 - "Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions"
Cohesion: 0.14
Nodes (11): Heinz Guderian (RTOS & DSP Lead), OpenOCD Dual-Core Trace Capture Syntax & Sizing Sweet Spot, Python SystemView Profiling & Visualization Suite (/analyzers), Standardized RTOS Timing Analysis Rules, Dual-Core Flight Loop Steady-State Benchmarks, Sub-Slice Fragmentation vs Full-Loop Boundary Timing Analysis, Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions, 1. Metadata & Scope (+3 more)

### Community 34 - "Firmware Coding Conventions & Architecture Rules"
Cohesion: 0.17
Nodes (12): Firmware Coding Conventions & Architecture Rules, Single-Line Control Flow & No-Space Bracing Style, Centralized Task Parameters Rule, Coding Conventions & Architecture Rules, Explicit Physical SI Units Rule, SystemView Tracing & RTOS Timing Conventions, FreeRTOS Task Scoping Architecture Rule, Max Ducson (Chief Mechatronics Engineer) (+4 more)

### Community 35 - "5-Layer Decoupled Firmware Architecture"
Cohesion: 0.15
Nodes (12): 5-Layer Decoupled Firmware Architecture, Layer 1: Platform HAL & Hardware Target Drivers, Layer 2: Sensor & Peripheral Drivers Layer, Layer 3: Middleware & Centralized Inter-Task IPC Layer, Layer 4: Flight Core & Algorithms Layer, Layer 5: Application Layer, Centralized Inter-Task IPC in Layer 3 (src/middleware/), 3. Dependency Mapping & Graphify Subgraph Analysis (+4 more)

### Community 36 - "PktImuFilt"
Cohesion: 0.33
Nodes (6): PktImuFilt, accel, gyro, _pad, timestamp_us, type

### Community 37 - "HAL_PWM"
Cohesion: 0.18
Nodes (8): HAL_PWM, get_freq_hz, get_resolution_bits, init_channel, init_timer, set_duty_ticks, set_pulse_us, stop_channel

### Community 39 - "FC_v01 Firmware Architecture & Structure Specification"
Cohesion: 0.11
Nodes (11): Deterministic Periodic Delay Policy via vTaskDelayUntil, ESP-DSP Hardware SIMD Primitive Integration, Layer 4 Matrix<T, ROWS, COLS> Template Architecture, ESP32-S3 RMT Hardware PPM RC Receiver Decoding, Layer 1 SIMD HAL Zero-Fallback Hardware Purity Rule, FC_v01 Firmware Architecture & Structure Specification, Centralized Task Parameter Enums in config.h, Firmware Structure Zettelkasten Traceability Index (+3 more)

### Community 40 - "HAL_Flash"
Cohesion: 0.09
Nodes (22): FlashStorageDriver, calculate_crc32, erase_namespace, erase_record, init, _initialized, read_record, write_record (+14 more)

### Community 41 - "Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision"
Cohesion: 0.20
Nodes (8): Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision, 1. Metadata & Scope, 2. Problem Statement & Context, 5. Cross-References & Traceability, Zettel 0038: Layer 3 OSAL & IPC Boundary Enforcement, Domain Struct Decoupling, and Interface Tasks Consolidation, 1. Metadata & Scope, 4. Cross-References & Traceability, Zettel 0039: On-Demand BLE OTA Firmware Flashing in DEVELOPMENT State via Dependency Inversion

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
Cohesion: 0.19
Nodes (12): 2. Problem Statement & Architectural Tension, 3. Core Architectural Decisions, Rule 1: Application Layer Exclusive Arming Authority, Rule 2: Layer 2 `MotorDriver` Internal Defense-in-Depth, Rule 3: Dual-Channel Mediator Architecture (Command Queue + Out-of-Band Notification), Rule 4: Two-Stage Verification in `motor_task` ("The Second Check"), Rule 5: Actuator Task Priority Elevation to Realtime (`4`), app_start() (+4 more)

### Community 48 - "PktImuRaw"
Cohesion: 0.33
Nodes (6): PktImuRaw, accel, gyro, _pad, timestamp_us, type

### Community 49 - "FlashRecordHeader"
Cohesion: 0.40
Nodes (5): FlashRecordHeader, crc32, magic, payload_len, schema_ver

### Community 50 - "ble_driver.cpp"
Cohesion: 0.18
Nodes (12): Layer 4 Task Identity Preservation (TASK_BLE_OTA_ID), 2. Problem Statement & Context, ble_ota_task(), get_char_by_mask(), is_ble_connected(), is_ble_stream_subscribed(), OtaControlCallbacks, OtaDataCallbacks (+4 more)

### Community 51 - "ESP32_PWM_LEDC"
Cohesion: 0.18
Nodes (10): ESP32_PWM_LEDC, _channel_initialized, _freq_hz, init_channel, init_timer, _resolution_bits, set_duty_ticks, set_pulse_us (+2 more)

### Community 53 - "Matrix"
Cohesion: 0.26
Nodes (8): dot(), hadamard(), inverse3x3(), Matrix, norm(), norm_sq(), normalize(), operator+()

### Community 55 - "0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling.md"
Cohesion: 0.06
Nodes (32): 3. Core Architectural Decisions, Rule 1: Dependency Inversion for Layer 2 BLE Driver Decoupling, Rule 2: On-Demand Task & Queue Spawning (`spawn_ble_ota_task`), Rule 3: Strict In-Flight Safety Lockout, Rule 4: Protocol Extension for Remote Reboot (`OTA_CMD_REBOOT`), 1. Metadata & Scope, 2. Problem Statement & Authority Inversion Violation, 3.1 Principle of Exclusive Layer 5 Authority (+24 more)

### Community 57 - "PktAttEuler"
Cohesion: 0.40
Nodes (5): PktAttEuler, euler, _pad, timestamp_us, type

### Community 58 - "LogFileHeader"
Cohesion: 0.18
Nodes (11): LogFileHeader, board_name, boot_timestamp_us, circular_head_sector, limit_policy, magic, max_file_size_bytes, registered_packets_mask (+3 more)

### Community 59 - "HAL_I2C"
Cohesion: 0.29
Nodes (6): HAL_I2C, probe, read_reg, read_reg_multi, write_reg, write_reg_multi

### Community 60 - "ESP32_I2C_ASYNC"
Cohesion: 0.14
Nodes (12): ESP32_I2C_ASYNC, _bus_handle, _frequency_hz, init, _initialized, _port, read_reg_async, read_reg_multi_async (+4 more)

### Community 61 - "PktMagRaw"
Cohesion: 0.40
Nodes (5): PktMagRaw, mag, _pad, timestamp_us, type

### Community 74 - "FC_v01 Architectural & Codebase Context Analysis Skill"
Cohesion: 0.25
Nodes (7): Graphify Knowledge Graph Rule, Graphify Query Before Grep Rule, FC_v01 Architectural & Codebase Context Analysis Skill, Codebase Graph Traversal Workflow, Zettelkasten Note Vault & ADR Scan Workflow, Workflow: analyze-context, analyze-context Execution Pipeline

### Community 79 - "Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout"
Cohesion: 0.11
Nodes (13): Sarah Thueson (Embedded Software Architect), 5-Layer Decoupled Firmware Architecture Decision, FreeRTOS Task Scoping Restricted to Layer 4 (core/) or Layer 5 (app/), Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout, Strict Sequential 6-Gate Pre-Flight Safety Evaluation Pipeline, Zettel 0031: Six-Gate Pre-Flight Safety Pipeline, Terminal Development Mode, Subsystem Task Encapsulation & Battery Enforcement Architecture, 1. Metadata & Scope, 4. Cross-References & Traceability (+5 more)

### Community 84 - "ESP32_PulseRX"
Cohesion: 0.05
Nodes (29): PulseSymbol, duration0_ns, duration1_ns, level0, level1, ESP32_PulseRX, init, initialized (+21 more)

### Community 110 - "BleTelemetryChannel"
Cohesion: 0.18
Nodes (7): BleTelemetryChannel, _last_ingested_ts, _last_send_tick, _mask_bit, _payload_buf, _payload_count, process()

### Community 129 - "ipc.cpp"
Cohesion: 0.21
Nodes (6): is_ble_ota_active(), set_ble_connected(), btn_hold_timer_callback(), FcBleServerCallbacks, onConnect, onDisconnect

### Community 159 - "AltitudeEKFEngine"
Cohesion: 0.07
Nodes (41): 5. RTOS Task Topology, Inter-Process Communication & Dependency Injection, 6. Verification & Traceability, 1. Metadata & Scope, 2. Core Architectural Principle: In-Flight Tracing Scope, Zettel 0041: SEGGER SystemView Tracing Scope & Exclusion of Maintenance and Calibration Tasks, PktAlt, _pad, state (+33 more)

### Community 161 - "SDCardDriver"
Cohesion: 0.11
Nodes (21): Rule 3: Lock-Free Snapshot Extraction & Timestamp Tracking, sd_logger_task(), SDCardDriver, _bytes_written, _cd_pin, close, _cs_pin, _file (+13 more)

### Community 172 - "Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent"
Cohesion: 0.08
Nodes (32): Empirical Flight Data Calibration Results & Universal Flight Analyzer Verification, ESP32-S3 128-Bit PIE SIMD Vector Dot Product Acceleration, Multivariate Analytical Gradient Derivation for Ellipsoid Optimization, Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent, Real-Time Flight Loop Matrix-Vector Ellipsoid Correction, 6-Parameter Symmetric Soft-Iron Matrix Formulation, 3D Rotated Ellipsoid to Unit-Sphere Mapping Formulation, Time-Sliced SIMD Adam Gradient Descent Optimizer for Ellipsoid Fitting (+24 more)

### Community 178 - "WS2812"
Cohesion: 0.06
Nodes (39): 3. Comprehensive OSAL & IPC Entity Audit, 4. Architectural Decisions & Execution Specification, Decision 1: Relocation of `led_task` to Unified `interface_tasks`, Decision 2: Executed Layer 3 IPC Refactoring & Domain Decoupling, status_led_task(), Color, b, g (+31 more)

### Community 205 - "Vec3"
Cohesion: 0.35
Nodes (9): lpf_apply(), Vec3, calib_apply_accel(), calib_apply_gyro(), calib_apply_mag(), calib_init(), calib_load_from_flash(), calib_reset_defaults() (+1 more)

### Community 226 - "FS_iA6B"
Cohesion: 0.19
Nodes (11): FS_iA6B, FS_iA6B::FS_iA6B(), init, is_failsafe, _last_channels, _last_frame_us, update, RcChannels (+3 more)

### Community 261 - "ble_ota_uploader.py"
Cohesion: 0.13
Nodes (4): BleOtaUploader, find_firmware_binary(), main(), run_reboot()

### Community 536 - "ESP32_I2C"
Cohesion: 0.18
Nodes (10): ESP32_I2C, ESP32_I2C::ESP32_I2C(), init, probe, read_reg, read_reg_multi, _scl_pin, _sda_pin (+2 more)

### Community 600 - "HAL_PulseRX"
Cohesion: 0.17
Nodes (10): 1. Metadata & Scope, 2. Problem Statement: Protocol Logic Bleed into Layer 1 HAL, 3. Proposed Future Decoupling Solution, Architectural Deficiencies, Zettel 0047: Layer 1 Hardware RMT PulseRX PPM Bleed & Layer 2 Protocol Decoupling Analysis, HAL_PulseRX, init, read_pulses (+2 more)

### Community 604 - "PktPower"
Cohesion: 0.25
Nodes (8): PktPower, current_ma, energy_mah, _pad, power_mw, timestamp_us, type, voltage_mv

### Community 645 - "Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers"
Cohesion: 0.11
Nodes (16): BLE Characteristic UUID Isolation Reference, Dual Execution Engine (SIMD vs Scalar LPF), IPC Clean Variable Naming & Ring Buffer Consolidation, Layer 4 DSP Filter & Calibration Scoping Rule, Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture, Harmonized Sensor Ring Buffer Architecture (accel, gyro, mag), Unification of Barometer Pressure Ring Buffer, Zettel 0022: Unified Pressure Ring Buffer, Filtered Sensor Variable Consolidation & Phantom Attitude Task Removal (+8 more)

### Community 656 - "LowPassFilter2ndOrder"
Cohesion: 0.15
Nodes (11): ButterworthFilter, LowPassFilter2ndOrder, _a, _b1, _b2, _initialized, _x_prev1, _x_prev2 (+3 more)

### Community 692 - "0037-threeStateVerticalVelocityAltitudeEkfAndBaroAccelFusionArchitecture.md"
Cohesion: 0.25
Nodes (7): 1. Metadata & Scope, 2. Problem Statement & Motivation, 3. Mathematical & Algorithmic Formulation, 4. Empirical Noise Covariance Grounding (`LOG_0032.BIN`), A. Coordinate Frame & Kinematic Acceleration (Z-Up), B. 3-State Extended Kalman Filter Equations, Zettel 0037: 3-State Vertical Velocity & Altitude Extended Kalman Filter (AltitudeEKFEngine), Dependency Injection & Layer 4 RTOS Task Architecture Specification

### Community 693 - "PktMagFilt"
Cohesion: 0.40
Nodes (5): PktMagFilt, mag, _pad, timestamp_us, type

### Community 694 - "Zettel 0044: System Physical Parameters, Mass Budget & Plant Inertia Estimation"
Cohesion: 0.33
Nodes (6): 1. Metadata & Scope, 2. Mass Breakdown & Measured Weight Budget, 3. Geometric & Frame Constants, 4. Derived Plant Inertia Estimation, 5. Cross-References & Traceability, Zettel 0044: System Physical Parameters, Mass Budget & Plant Inertia Estimation

### Community 695 - "Zettel 0027: Pre-Flight Sensor Data Stream Health Check, Multi-Rate Latency Verification & FSM Transition Gate Architecture"
Cohesion: 0.20
Nodes (7): Multi-Rate Timestamp Freshness and Inter-Arrival Latency Verification, Physical Domain and Numerical Sanity Boundary Validation, Two-Stage Temporal Stability Latching with Visual LED Failure Indication, Zettel 0027: Pre-Flight Sensor Data Stream Health Check, Multi-Rate Latency Verification & FSM Transition Gate Architecture, Pre-Flight Battery Health Verification and LiPo Cell Auto-Detection, INA226 Shunt Resistor Parameterization and Fixed-Point Millis Scale Scaling, Zettel 0028: INA226 Power Module I2C1 DAQ, Shunt Resistor Calibration, Millis Scale Metrics & Battery Health Monitoring Architecture

### Community 696 - "RingBuffer"
Cohesion: 0.23
Nodes (3): RingBuffer, _buffer, _head

### Community 697 - "HAL_I2C_ASYNC"
Cohesion: 0.22
Nodes (6): HAL_I2C_ASYNC, init, read_reg_async, read_reg_multi_async, write_reg_async, write_reg_multi_async

### Community 698 - "LowPassFilter1stOrder"
Cohesion: 0.22
Nodes (7): Tustin Bilinear 1st-Order Low-Pass Filter State (LowPassFilter1stOrder), LowPassFilter1stOrder, a, b, initialized, x_prev, y_prev

### Community 699 - "BleVec3Packet"
Cohesion: 0.50
Nodes (3): BleVec3Packet, data, timestamp

### Community 700 - "PktRc"
Cohesion: 0.20
Nodes (9): 3. Core Architectural Decisions, 4. Architectural Summary, Rule 1: Binary Packet Specification (`PktRc`) & 4-Byte Struct Alignment, Rule 2: Centralized Packet Type & Bitmask Registration, PktRc, channels, _pad, timestamp_us (+1 more)

### Community 701 - "PktBaroFilt"
Cohesion: 0.50
Nodes (4): PktBaroFilt, pressure, timestamp_us, type

### Community 704 - "PktBaroRaw"
Cohesion: 0.50
Nodes (4): PktBaroRaw, pressure, timestamp_us, type

## Knowledge Gaps
- **400 isolated node(s):** `type`, `timestamp_us`, `_pad`, `gyro`, `accel` (+395 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 1244 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **647 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` connect `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` to `Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions`, `app.cpp`, `5-Layer Decoupled Firmware Architecture`, `Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture`, `Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing`, `FC_v01 Firmware Architecture & Structure Specification`, `Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers`, `Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision`, `FC_v01 Architectural & Codebase Context Analysis Skill`, `BLE Telemetry 500 Hz Non-Blocking Data Flow`, `HAL_SPI`, `0037-threeStateVerticalVelocityAltitudeEkfAndBaroAccelFusionArchitecture.md`, `0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling.md`, `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision`, `Zettel 0027: Pre-Flight Sensor Data Stream Health Check, Multi-Rate Latency Verification & FSM Transition Gate Architecture`, `HAL_PulseRX`, `Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage`?**
  _High betweenness centrality (0.052) - this node is a cross-community bridge._
- **Why does `Vec3` connect `Vec3` to `PktImuFilt`, `Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture`, `FC_v01 Firmware Architecture & Structure Specification`, `Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent`, `PktImuRaw`, `LowPassFilter2ndOrder`, `WS2812`, `PktMagFilt`, `Matrix`, `PktAttEuler`, `LowPassFilter1stOrder`, `BleVec3Packet`, `PktMagRaw`, `AltitudeEKFEngine`?**
  _High betweenness centrality (0.038) - this node is a cross-community bridge._
- **Why does `HAL_PWM` connect `HAL_PWM` to `app.cpp`, `3. Core Architectural Decisions`, `cstdint`, `ESP32_PWM_LEDC`, `MotorDriver`?**
  _High betweenness centrality (0.026) - this node is a cross-community bridge._
- **Are the 2 inferred relationships involving `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` (e.g. with `Zettelkasten Decision Notes Rules` and `control_tasks.cpp`) actually correct?**
  _`Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` has 2 INFERRED edges - model-reasoned connections that need verification._
- **What connects `type`, `timestamp_us`, `_pad` to the rest of the system?**
  _400 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `ble_packet.h` be split into smaller, more focused modules?**
  _Cohesion score 0.08695652173913043 - nodes in this community are weakly interconnected._
- **Should `Zettel 0020: Finite State Machine Backbone, Pre-Flight Sensor Checks & Active Calibration Architecture` be split into smaller, more focused modules?**
  _Cohesion score 0.10574712643678161 - nodes in this community are weakly interconnected._