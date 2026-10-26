# Graph Report - FC_v01  (2026-10-22)

## Corpus Check
- 455 files · ~492,983 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 127 file(s) not represented in the graph (top: .S 92, .defaults 15, (none) 11)

## Summary
- 2090 nodes · 4489 edges · 140 communities (92 shown, 48 thin omitted)
- Extraction: 89% EXTRACTED · 11% INFERRED · 0% AMBIGUOUS · INFERRED: 479 edges (avg confidence: 0.84)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `1df59b29`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- dsps_fft2r_sc16_ansi.c
- WS2812
- Mat
- esp_log.h
- Image3DMatrix
- ESP32_I2C_ASYNC
- INA226
- Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage
- BMP180
- FC_v01 Firmware Architecture & Structure Specification
- dsps_fird_init_s16.c
- config.h
- app.cpp
- Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent
- Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision
- ESP32_SPI
- fsm.cpp
- Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision
- HMC5883L
- math
- dsp_err.h
- mat.cpp
- test_mat_sub_f32.cpp
- SDCardDriver
- Zettel 0030: ESP32-S3 Hardware RMT PPM Receiver Decoding (Flysky FS-iA6B), Layer 1-4 Decoupled Architecture & BLE Real-Time Telemetry Pipeline
- ipc.h
- AttitudeEKF
- Attitude Outer Loop Task (100Hz)
- ekf
- MPU6050
- Matrix
- BlePowerPacket
- LowPassFilter1stOrder
- kalman_filter_demo.cpp
- HAL_PulseRX
- ekf_imu13states
- ble_packet.h
- ekf_imu13states.cpp
- HAL_Flash
- onConnect
- init_display.c
- ble_driver.cpp
- BLE Telemetry 500 Hz Non-Blocking Data Flow
- ESP-DSP Mat Matrix 3D Projection & Transformation
- mpu6050.cpp
- PktAttEuler
- ESP-DSP Basic Math Example Guide
- dsps_fft4real_main.c
- main.cpp
- ChannelTracker
- azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp
- audio_amp_main.c
- RingBuffer
- Firmware Coding Conventions & Architecture Rules
- IMU 9-DOF Sensor Fusion via Kalman Filter
- ESP-DSP Library Overview
- ESP-DSP Complex FFT Example Guide
- BleTelemetryChannel
- ESP-DSP SIMD Assembly Optimization
- 3d_kalman_demo.cpp
- test_dsps_conv_f32_ansi.c
- LogFileHeader
- dsps_resampler_ph.c
- m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp
- m_test_data_s
- Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions
- Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers
- FC_v01 Architectural & Codebase Context Analysis Skill
- Rect
- sd_logger_task.cpp
- Zettel 0015: BLE OTA Lifecycle Architecture, Resource Stewardship & Host Stack Decoupling
- SensorCalibrationData
- Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture
- ESP-DSP Examples Overview and Structure
- ESP-DSP FIR Filter Example Guide
- accel_gyro_daq_task
- HAL_SPI
- 5-Layer Decoupled Firmware Architecture
- dspm_mult_ex_f32_aexx_functionality_in_cycle
- CPU Logo Converted 3D Matrix Image
- test_dsps_memcpy_memset.c
- dsps_dct_f32.c
- Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout
- ESP-DSP Applications Layout
- ESP-DSP Library Component Specification
- PktImuFilt
- PktImuRaw
- FlashRecordHeader
- dsps_h_gen
- M5Stack CoreS3 Kalman Demo Component Registration
- build_all.sh
- aes3_tie_log.c
- dspi_dotprod_off_s16_ansi
- dspi_dotprod_off_s8_ansi
- dspi_dotprod_off_u16_ansi
- dspi_dotprod_off_u8_ansi
- dspi_dotprod_s16_ansi
- dspi_dotprod_s8_ansi
- dspi_dotprod_u16_ansi
- dspi_dotprod_u8_ansi
- dspi_dotprod_f32_ansi
- dspi_dotprod_off_f32_ansi
- CalibReportMsg
- dsps_biquad_f32_ansi
- dsps_corr_f32_ansi.c
- dsps_dotprod_s16_ansi.c
- dsps_dp_s8_ansi.c
- dsps_dotprod_f32_ansi.c
- dsps_dotprode_f32_ansi.c
- dsps_fird_s16_ansi.c
- dsps_fird_f32_ansi.c
- dsps_biquad_sf32_ansi.c
- dsps_sqrt_f32_ansi.c
- dsps_add_s16_ansi.c
- dsps_add_s8_ansi.c
- dsps_add_f32_ansi.c
- dsps_addc_f32_ansi.c
- dsps_mul_s8_ansi.c
- dsps_mul_f32_ansi.c
- dsps_mulc_s16_ansi.c
- dsps_sub_s16_ansi.c
- dsps_sub_s8_ansi.c
- dsps_sub_f32_ansi.c
- dspm_add_f32_ansi.c
- dspm_addc_f32_ansi.c
- dspm_mult.h
- Vec3
- dspm_mult_ex_f32_ansi.c
- PowerMeasurement
- dspm_mulc_f32_ansi.c
- dspm_sub_f32_ansi.c
- FC_v01 CMake Project
- I2C1 Bus Mutex Guard
- Spectrum Box Lite CMake Project Definition
- dspi_conv_f32_ansi
- main

## God Nodes (most connected - your core abstractions)
1. `Mat` - 98 edges
2. `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` - 40 edges
3. `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision` - 35 edges
4. `INA226` - 33 edges
5. `Vec3` - 32 edges
6. `MPU6050` - 30 edges
7. `Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision` - 29 edges
8. `ekf` - 26 edges
9. `Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers` - 25 edges
10. `FC_v01 Firmware Architecture & Structure Specification` - 24 edges

## Surprising Connections (you probably didn't know these)
- `EKF Process & UpdateRefMeasurement Recursion` --semantically_similar_to--> `EKF Roll & Pitch Task (100Hz)`  [INFERRED] [semantically similar]
  managed_components/espressif__esp-dsp/modules/kalman/ekf_imu13states/docs/README.md → documents/RTOS Architecture.pdf
- `System Life Cycle Document (Placeholder)` --conceptually_related_to--> `dspm_mult_ex_f32_aexx_functionality_in_cycle()`  [INFERRED]
  documents/System Life Cycle.md → managed_components/espressif__esp-dsp/modules/matrix/mul/test/test_mmult_ex_f32_aexx.cpp
- `Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (sd_logger_task)` --implements--> `sd_logger_task()`  [EXTRACTED]
  documents/notes/0013-sdCardSpiStorageAndFlightLoggingArchitecture.md → src/core/blackbox/sd_logger_task.cpp
- `Layer 4 Matrix<T, ROWS, COLS> Template Architecture` --implements--> `Vec3`  [EXTRACTED]
  documents/Firmware Structure.md → src/core/math/matrix.h
- `Unified Matrix<T, ROWS, COLS> Template Architecture` --implements--> `Vec3`  [EXTRACTED]
  documents/notes/0004-simdMultiTypeSupportAndBuildIntegration.md → src/core/math/matrix.h

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Cascaded Attitude Control Architecture** — documents_rtos_architecture_attitude_outer_loop, documents_rtos_architecture_attitude_inner_loop, documents_rtos_architecture_motor_mixer [EXTRACTED 0.95]
- **IMU Multi-Sensor Fusion Pipeline** — documents_rtos_architecture_accel_gyro_calib_filter, documents_rtos_architecture_mag_calib_filter, documents_rtos_architecture_ekf_roll_pitch, documents_rtos_architecture_ekf_yaw [EXTRACTED 0.95]
- **Sensor Data Acquisition and Ring Buffer Decoupling** — documents_rtos_architecture_accel_gyro_daq, documents_rtos_architecture_mag_daq, documents_rtos_architecture_baro_daq, documents_rtos_architecture_ring_buffer_newest_index_rule [EXTRACTED 0.95]
- **ESP-DSP Azure IoT Kit 3D Graphics Visualization Flow** — managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_3d_graphics_3d_graphics_demo_animation, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_3d_graphics_3d_graphics_matrix_rendering, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_3d_graphics_readme_app_guide [EXTRACTED 1.00]
- **ESP-DSP Azure IoT Kit Kalman Filter Visualization Flow** — managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_kalman_filter_kalman_filter_demo_animation, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_kalman_filter_kalman_filter_attitude_estimation, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_kalman_filter_readme_imu_sensor_fusion [EXTRACTED 1.00]
- **ESP-DSP Core Digital Signal Processing Primitives** — managed_components_espressif__esp_dsp_readme_matrix_operations, managed_components_espressif__esp_dsp_readme_fft, managed_components_espressif__esp_dsp_readme_iir_filter, managed_components_espressif__esp_dsp_readme_fir_filter, managed_components_espressif__esp_dsp_readme_vector_math, managed_components_espressif__esp_dsp_readme_kalman_filter [EXTRACTED 1.00]
- **Dual-Microphone Real-Time Audio Visualizer Pipeline** — managed_components_espressif__esp_dsp_applications_spectrum_box_lite_readme_overview, managed_components_espressif__esp_dsp_applications_spectrum_box_lite_readme_audio_pipeline, managed_components_espressif__esp_dsp_applications_spectrum_box_lite_main_idf_component_lvgl_dep [EXTRACTED 1.00]
- **ESP-DSP 3D Matrix Transformation and Projection Pipeline** — managed_components_espressif__esp_dsp_applications_azure_board_apps_readme_mat_3d_projection, managed_components_espressif__esp_dsp_applications_azure_board_apps_graphics_img_to_3d_matrix_example_readme_img_to_3d_pipeline, managed_components_espressif__esp_dsp_applications_m5stack_core_s3_apps_3d_graphics_main_cmakelists_component [INFERRED 0.95]
- **ESP-DSP Digital Filtering and Estimation Examples** — managed_components_espressif__esp_dsp_examples_fir_readme_windowed_sinc_fir, managed_components_espressif__esp_dsp_examples_iir_readme_biquad_iir_filter, managed_components_espressif__esp_dsp_examples_kalman_readme_13_state_ekf [INFERRED 0.95]
- **ESP-DSP IMU Kalman Filter Sensor Fusion Demonstrations** — managed_components_espressif__esp_dsp_readme_kalman_filter, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_kalman_filter_readme_imu_sensor_fusion, managed_components_espressif__esp_dsp_applications_m5stack_core_s3_apps_kalman_filter_readme_bmi270_bmm150_fusion [INFERRED 0.95]
- **ESP-DSP SIMD Vector and Matrix Operations Examples** — managed_components_espressif__esp_dsp_examples_dotprod_readme_vector_dot_product, managed_components_espressif__esp_dsp_examples_basic_math_readme_vector_multiplication, managed_components_espressif__esp_dsp_examples_conv2d_readme_2d_convolution, managed_components_espressif__esp_dsp_examples_matrix_cmakelists_matrix_acceleration, managed_components_espressif__esp_dsp_readme_simd_assembly_optimization [INFERRED 0.95]
- **ESP-DSP Frequency Analysis and FFT Windowing Examples** — managed_components_espressif__esp_dsp_examples_fft_readme_complex_fft_split, managed_components_espressif__esp_dsp_examples_fft4real_readme_real_fft, managed_components_espressif__esp_dsp_examples_fft_window_readme_windowing_functions, managed_components_espressif__esp_dsp_readme_fft [INFERRED 0.95]

## Communities (140 total, 48 thin omitted)

### Community 1 - "dsps_fft2r_sc16_ansi.c"
Cohesion: 0.06
Nodes (46): microphone_read_task(), dsp_is_power_of_two(), dsp_power_of_two(), dsps_bit_rev_sc16_ansi(), dsps_cplx2real_sc16_ansi(), dsps_cplx2reC_sc16(), dsps_fft2r_init_sc16(), dsps_fft2r_sc16_ansi_() (+38 more)

### Community 2 - "WS2812"
Cohesion: 0.05
Nodes (44): status_led_task(), Color, b, g, r, get_palette_color(), WS2812, brightness (+36 more)

### Community 3 - "Mat"
Cohesion: 0.05
Nodes (49): ekf::dFdq(), ekf::dFdq_inv(), ekf::eul2rotm(), ekf::qProduct(), ekf::quat2eul(), ekf::quat2rotm(), ekf::rotm2eul(), ekf::rotm2quat() (+41 more)

### Community 5 - "Image3DMatrix"
Cohesion: 0.05
Nodes (9): convert_img(), create_matrix(), fromat_comas_spaces(), Image3DMatrix, main(), ouptput_files_gen(), BleOtaUploader, find_firmware_binary() (+1 more)

### Community 6 - "ESP32_I2C_ASYNC"
Cohesion: 0.09
Nodes (22): ESP32_I2C_ASYNC, ESP32_I2C_ASYNC::ESP32_I2C_ASYNC(), _frequency_hz, init, _initialized, _port, read_reg_async, read_reg_multi_async (+14 more)

### Community 7 - "INA226"
Cohesion: 0.09
Nodes (32): power_daq_task(), INA226, calibrate, _current_lsb_ma, _dev_addr, get_all_data, get_all_data_raw, INA226::INA226() (+24 more)

### Community 8 - "Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage"
Cohesion: 0.09
Nodes (18): Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (sd_logger_task), 4-Digit Incremental File Scheme (LOG_0001.BIN) & LogFileHeader, Unified HAL_SPI Abstraction & Shared Bus Decoupling (Layer 1), Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage, 512-Byte Self-Describing Sector-Aligned Binary Log Header (LogFileHeader), Hardware USER Button 3-Second Hold Graceful Shutdown Protocol, Zettel 0018: MicroSD Flight Logging Architecture, POSIX Metadata Sync, User Button Shutdown & Binary Log Specification, Multi-Rate Binary Telemetry Timestamp Alignment and Interpolation (+10 more)

### Community 9 - "BMP180"
Cohesion: 0.08
Nodes (30): baro_daq_task(), BMP180, _b5, BMP180::BMP180(), calculate_b5, calculate_pressure, calculate_temperature, _calib (+22 more)

### Community 10 - "FC_v01 Firmware Architecture & Structure Specification"
Cohesion: 0.15
Nodes (8): Deterministic Periodic Delay Policy via vTaskDelayUntil, ESP-DSP Hardware SIMD Primitive Integration, Layer 4 Matrix<T, ROWS, COLS> Template Architecture, ESP32-S3 RMT Hardware PPM RC Receiver Decoding, Layer 1 SIMD HAL Zero-Fallback Hardware Purity Rule, FC_v01 Firmware Architecture & Structure Specification, Centralized Task Parameter Enums in config.h, Firmware Structure Zettelkasten Traceability Index

### Community 11 - "dsps_fird_init_s16.c"
Cohesion: 0.06
Nodes (16): dsps_16_array_rev(), dsps_fird_init_s16(), dsps_fird_s16_aexx_free(), dsps_firmr_init_s16(), dsps_fir_f32_ansi(), dsps_fir_f32_free(), dsps_fir_init_f32(), dsps_firmr_init_f32() (+8 more)

### Community 12 - "config.h"
Cohesion: 0.11
Nodes (12): HAL_I2C_ASYNC, init, read_reg_async, read_reg_multi_async, write_reg_async, write_reg_multi_async, HAL_I2C, probe (+4 more)

### Community 13 - "app.cpp"
Cohesion: 0.07
Nodes (4): user_button_is_pressed(), btn_hold_timer_callback(), ipc_pop_btn_events(), user_btn_isr()

### Community 14 - "Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent"
Cohesion: 0.16
Nodes (12): Empirical Flight Data Calibration Results & Universal Flight Analyzer Verification, ESP32-S3 128-Bit PIE SIMD Vector Dot Product Acceleration, Multivariate Analytical Gradient Derivation for Ellipsoid Optimization, Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent, Real-Time Flight Loop Matrix-Vector Ellipsoid Correction, 6-Parameter Symmetric Soft-Iron Matrix Formulation, 3D Rotated Ellipsoid to Unit-Sphere Mapping Formulation, Time-Sliced SIMD Adam Gradient Descent Optimizer for Ellipsoid Fitting (+4 more)

### Community 15 - "Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision"
Cohesion: 0.10
Nodes (21): Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision, Centralized Enum Task Parameter Declarations (include/config.h), Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing, Centralized UUID Service & Characteristic Series (include/config.h), Dedicated Task Mapping in src/core/telemetry/ble_telem_tasks.cpp, Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision, Centralized Frequency Floor (BLE_TELEM_MIN_PACKET_FREQ_HZ = 10), Zettel 0010: BLE Minimum Packet Frequency and Dual-Trigger Transmission Protocol (+13 more)

### Community 16 - "ESP32_SPI"
Cohesion: 0.15
Nodes (16): ESP32_SPI, ESP32_SPI::ESP32_SPI(), get_free_bytes, get_total_bytes, init, _initialized, is_card_inserted, is_fatfs_mounted (+8 more)

### Community 17 - "fsm.cpp"
Cohesion: 0.17
Nodes (22): check_match_policy(), check_passive_sensor_calibration(), check_platform_tilt(), check_power_initial_condition(), check_rc_controller_signal(), check_stationary_condition(), evaluate_state_transitions(), fsm_clear_event() (+14 more)

### Community 18 - "Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision"
Cohesion: 0.11
Nodes (20): Centralized Inter-Task Communication in src/middleware/ipc.h and ipc.cpp, Deterministic Periodic Task Timing via vTaskDelayUntil, Structured Enum-Based Task Parameters (include/config.h), Strict 4-Level RTOS Priority Architecture, Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision, Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule, ESP-DSP Multi-Type SIMD Integration, Layer 1 hal_simd Multi-Type Hardware SIMD Acceleration (float, int16_t, int8_t) (+12 more)

### Community 19 - "HMC5883L"
Cohesion: 0.10
Nodes (18): Real-Time Flight Loop Calibration (m_calib = W * (m_raw - v0)), mag_daq_task(), HMC5883L, _dev_addr, get_all_data, get_all_data_raw, HMC5883L::HMC5883L(), init (+10 more)

### Community 20 - "math"
Cohesion: 0.05
Nodes (6): dsps_dctiv_f32(), dsps_dstiv_f32(), dsps_bit_rev_lookup_fc32_ae32(), dsps_cplx2real_fc32_ae32_(), dsps_firmr_s16_ansi(), dsps_firmr_f32_ansi()

### Community 21 - "dsp_err.h"
Cohesion: 0.07
Nodes (13): "dspm_add_f32_ae32 benchmark", "dspm_add_f32_ae32 functionality", "dspm_add_f32_ansi benchmark", "dspm_add_f32_ansi functionality", "dspm_addc_f32_ansi benchmark", "dspm_mult_ex_f32_aexx benchmark", "dspm_mult_ex_f32_aexx functionality", "dspm_mult_ex_f32_ansi benchmark" (+5 more)

### Community 22 - "mat.cpp"
Cohesion: 0.07
Nodes (19): augment, expHelper, eye, swapRows, Mat::augment(), Mat::Copy(), Mat::CopyHead(), Mat::det() (+11 more)

### Community 23 - "test_mat_sub_f32.cpp"
Cohesion: 0.22
Nodes (25): Copy, Get, getROI, inverse, pinv, test_assert_check_area_mat_mat(), test_assert_equal_mat_mat(), "Mat class matrix subset" (+17 more)

### Community 24 - "SDCardDriver"
Cohesion: 0.14
Nodes (19): append_packet(), sd_logger_task(), SDCardDriver, _bytes_written, _cd_pin, close, _cs_pin, _file (+11 more)

### Community 25 - "Zettel 0030: ESP32-S3 Hardware RMT PPM Receiver Decoding (Flysky FS-iA6B), Layer 1-4 Decoupled Architecture & BLE Real-Time Telemetry Pipeline"
Cohesion: 0.22
Nodes (7): Uniform Consumer Snapshot Array Sizing Matching Ring Buffer Capacity, Uniform Ring Buffer Sizing Across Layer 3 Middleware IPC, Zettel 0029: Unification of Inter-Task Ring Buffer Capacity Constant, 28-Byte Binary Packet Structure (BleRcPacket) for Real-Time Ground Telemetry, Pre-Flight RC Pulse Signal Validation and Failsafe Detection Engine, Hardware Remote Control Peripheral (RMT) Pulse-Width Demodulation with Zero ISR Jitter, Zettel 0030: ESP32-S3 Hardware RMT PPM Receiver Decoding (Flysky FS-iA6B), Layer 1-4 Decoupled Architecture & BLE Real-Time Telemetry Pipeline

### Community 26 - "ipc.h"
Cohesion: 0.17
Nodes (7): FlashStorageDriver, LedCommand, color, pattern, OtaDataChunk, data, length

### Community 28 - "AttitudeEKF"
Cohesion: 0.19
Nodes (17): AttitudeEKF, AttitudeEKF::AttitudeEKF(), _earth_bx, _earth_bz, get_euler_angles, get_gyro_bias, get_quaternion, normalize_quaternion (+9 more)

### Community 29 - "Attitude Outer Loop Task (100Hz)"
Cohesion: 0.12
Nodes (19): Accel Gyro Calib & Filter Task (250Hz), Accel Gyro DAQ Task (500Hz), Altitude & Vz Estimator Task (100Hz), Attitude Inner Loop Task (250Hz, 4 Pri), Attitude Outer Loop Task (100Hz), Barometer Calib & Filter Task, Barometer DAQ Task, EKF Roll & Pitch Task (100Hz) (+11 more)

### Community 30 - "ekf"
Cohesion: 0.10
Nodes (21): ekf::RungeKutta(), ekf, CovariancePrediction, dFdq, dFdq_inv, eul2rotm, HP, Init (+13 more)

### Community 31 - "MPU6050"
Cohesion: 0.10
Nodes (13): MPU6050, _accel_raw_x, _accel_raw_y, _accel_raw_z, _accel_sens, _dev_addr, _gyro_raw_x, _gyro_raw_y (+5 more)

### Community 32 - "Matrix"
Cohesion: 0.26
Nodes (8): dot(), hadamard(), inverse3x3(), Matrix, norm(), norm_sq(), normalize(), operator+()

### Community 33 - "BlePowerPacket"
Cohesion: 0.25
Nodes (6): BlePowerPacket, current_ma, energy_mah, power_mw, timestamp, voltage_mv

### Community 34 - "LowPassFilter1stOrder"
Cohesion: 0.15
Nodes (16): Tustin Bilinear 1st-Order Low-Pass Filter State (LowPassFilter1stOrder), LowPassFilter1stOrder, b, initialized, x_prev, y_prev, lpf_apply(), lpf_init() (+8 more)

### Community 35 - "kalman_filter_demo.cpp"
Cohesion: 0.18
Nodes (14): app_fbm320_init(), app_mag3110_init(), app_main(), app_ssd1306_init(), display_3d_image(), draw_3d_image(), get_initial_pressure(), get_pressure_task() (+6 more)

### Community 36 - "HAL_PulseRX"
Cohesion: 0.09
Nodes (21): PktRc, channels, _pad, timestamp_us, type, FS_iA6B, FS_iA6B::FS_iA6B(), init (+13 more)

### Community 37 - "ekf_imu13states"
Cohesion: 0.11
Nodes (17): image_3d_matrix_kalman_s, ekf13, matrix_len, image_3d_matrix_s, matrix_len, ekf_imu13states::Test(), ekf_imu13states, accel0 (+9 more)

### Community 38 - "ble_packet.h"
Cohesion: 0.14
Nodes (9): BleAttQuatPacket, q, timestamp, BleBaroPacket, pressure, timestamp, BleRcPacket, ch (+1 more)

### Community 39 - "ekf_imu13states.cpp"
Cohesion: 0.14
Nodes (16): app_main(), app_main(), ekf::CovariancePrediction(), ekf_imu13states::Init(), ekf_imu13states::LinearizeFG(), ekf_imu13states::StateXdot(), ekf_imu13states::TestFull(), ekf_imu13states::UpdateRefMeasurement() (+8 more)

### Community 40 - "HAL_Flash"
Cohesion: 0.09
Nodes (22): FlashStorageDriver, calculate_crc32, erase_namespace, erase_record, init, _initialized, read_record, write_record (+14 more)

### Community 41 - "onConnect"
Cohesion: 0.32
Nodes (5): is_ble_ota_active(), set_ble_connected(), FcBleServerCallbacks, onConnect, onDisconnect

### Community 42 - "init_display.c"
Cohesion: 0.21
Nodes (9): bsp_display_lcd_init(), bsp_enable_feature(), bsp_i2c_init(), bsp_spi_init(), display_init(), dsp_display_brightness_init(), dsp_display_new(), dsp_display_start_with_config() (+1 more)

### Community 43 - "ble_driver.cpp"
Cohesion: 0.21
Nodes (12): Layer 4 Task Identity Preservation (TASK_BLE_OTA_ID), app_suspend_all_tasks(), ble_ota_task(), get_char_by_mask(), is_ble_connected(), is_ble_stream_subscribed(), OtaControlCallbacks, OtaDataCallbacks (+4 more)

### Community 44 - "BLE Telemetry 500 Hz Non-Blocking Data Flow"
Cohesion: 0.33
Nodes (6): BLE Telemetry 500 Hz Non-Blocking Data Flow, Custom Dual OTA Partition Scheme (8MB Layout), Project: FC_v01 Firmware & 5-Layer RTOS Architecture, 5-Layer Decoupled RTOS Architecture Project Scope, GyroTelemetrySample IPC Data Contract, Centralized Interface Contracts (config.h, ipc.h, ble_driver.h)

### Community 45 - "ESP-DSP Mat Matrix 3D Projection & Transformation"
Cohesion: 0.13
Nodes (17): ESP32-Azure IoT Kit 3D Graphics Demo Animation, 3D Matrix Transformation and Projection on LCD, esp-dsp-azure-board-app-3d-graphics Project, Azure 3D Graphics Main Component, Azure 3D Graphics IDF Component Manifest, Azure 3D Graphics Demo Application Guide, ImgTo3D Generation Example Guide, ImgTo3D 2D to 3D Matrix Generator Pipeline (+9 more)

### Community 46 - "mpu6050.cpp"
Cohesion: 0.16
Nodes (9): enable_bypass, enable_sleep, MPU6050::get_all_data_async(), get_all_data_raw, init, MPU6050::MPU6050(), set_accel_range, set_clock_source (+1 more)

### Community 47 - "PktAttEuler"
Cohesion: 0.50
Nodes (4): PktAttEuler, euler, timestamp_us, type

### Community 48 - "ESP-DSP Basic Math Example Guide"
Cohesion: 0.15
Nodes (11): ESP-DSP Basic Math Example Project, ESP-DSP Basic Math Main Component, ESP-DSP Basic Math Component Dependency Manifest, ESP-DSP Basic Math Example Guide, ESP-DSP Vector Multiplication Operations (dsps_mul_f32, dsps_mulc_f32), ESP-DSP Dot Product Example Project, ESP-DSP Dot Product Main Component, ESP-DSP Dot Product Component Dependency Manifest (+3 more)

### Community 49 - "dsps_fft4real_main.c"
Cohesion: 0.06
Nodes (28): convert_to_rgb(), image_display_task(), spectrum2d_picture(), spectrum2d_picture_init(), app_main(), process_and_show(), app_main(), app_main() (+20 more)

### Community 50 - "main.cpp"
Cohesion: 0.14
Nodes (9): Layer 3: Middleware & Centralized Inter-Task IPC Layer, Centralized Inter-Task IPC in Layer 3 (src/middleware/), app_start(), fsm_init(), user_button_init(), app_main(), init_ipc(), sysview_init_markers() (+1 more)

### Community 51 - "ChannelTracker"
Cohesion: 0.50
Nodes (3): ChannelTracker, has_logged, last_ts

### Community 52 - "azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.15
Nodes (15): ESP-DSP Changelog Document, app_main(), app_ssd1306_init(), dispaly_esp_text(), display_3d_image(), draw_3d_image_task(), init_3d_matrix_struct(), init_perspective_matrix() (+7 more)

### Community 53 - "audio_amp_main.c"
Cohesion: 0.14
Nodes (17): audio_process_task(), audio_read_task(), buttons_process_task(), convert_float2short(), convert_short2float(), digitalLimiter(), dsps_biquad_gen_allpass180_f32(), dsps_biquad_gen_allpass360_f32() (+9 more)

### Community 55 - "RingBuffer"
Cohesion: 0.23
Nodes (3): RingBuffer, _buffer, _head

### Community 56 - "Firmware Coding Conventions & Architecture Rules"
Cohesion: 0.17
Nodes (12): Firmware Coding Conventions & Architecture Rules, Single-Line Control Flow & No-Space Bracing Style, Centralized Task Parameters Rule, Coding Conventions & Architecture Rules, Explicit Physical SI Units Rule, SystemView Tracing & RTOS Timing Conventions, FreeRTOS Task Scoping Architecture Rule, Max Ducson (Chief Mechatronics Engineer) (+4 more)

### Community 57 - "IMU 9-DOF Sensor Fusion via Kalman Filter"
Cohesion: 0.18
Nodes (12): esp-dsp-azure-board-app-kalman-filter Project, Real-Time 3D Attitude Estimation and Sensor Fusion Display, ESP32-Azure IoT Kit Kalman Filter IMU Demo Animation, Azure Kalman Filter Main Component, Azure Kalman Filter IDF Component Manifest, Azure Kalman Filter Demo Application Guide, IMU 9-DOF Sensor Fusion via Kalman Filter, esp-dsp-m5stack-3d-kalman Project (+4 more)

### Community 58 - "ESP-DSP Library Overview"
Cohesion: 0.20
Nodes (11): ESP-DSP GitLab Pre-Commit CI Job, ESP-DSP Pre-Commit Hook Configurations, LyraT Audio App Project, LyraT Audio Main Component, LyraT Audio IDF Component Manifest, LyraT Audio Processing Demo Guide, ESP-DSP IIR Biquad Equalizer & Digital Limiter, ESP-DSP Contribution Guide (+3 more)

### Community 59 - "ESP-DSP Complex FFT Example Guide"
Cohesion: 0.15
Nodes (11): ESP-DSP Complex FFT Example Project, ESP-DSP Complex FFT Main Component, ESP-DSP Complex FFT Component Dependency Manifest, Complex FFT with Real Spectrum Splitting (dsps_cplx2reals_split), ESP-DSP Complex FFT Example Guide, ESP-DSP FFT Window Example Project, ESP-DSP FFT Window Main Component, ESP-DSP FFT Window Component Dependency Manifest (+3 more)

### Community 60 - "BleTelemetryChannel"
Cohesion: 0.18
Nodes (7): BleTelemetryChannel, _last_ingested_ts, _last_send_tick, _mask_bit, _payload_buf, _payload_count, process()

### Community 61 - "ESP-DSP SIMD Assembly Optimization"
Cohesion: 0.15
Nodes (10): ESP-DSP 2D Convolution Example Project, ESP-DSP 2D Convolution Main Component, ESP-DSP 2D Convolution Component Dependency Manifest, ESP-DSP 2D Convolution Image Processing (dspi_conv_f32), ESP-DSP 2D Convolution Example Guide, ESP-DSP Real Input FFT Example Project, ESP-DSP Real Input FFT Main Component, ESP-DSP Real Input FFT Component Dependency Manifest (+2 more)

### Community 62 - "3d_kalman_demo.cpp"
Cohesion: 0.29
Nodes (9): app_init(), app_main(), init_3d_matrix_struct(), read_bmi270_data(), read_bmi270_reg(), read_bmm150_data(), write_bmi270_data(), write_bmi270_reg() (+1 more)

### Community 65 - "test_dsps_conv_f32_ansi.c"
Cohesion: 0.20
Nodes (3): dsps_ccorr_f32_ansi(), dsps_conv_f32_ansi(), dsps_conv_f32_ref()

### Community 66 - "LogFileHeader"
Cohesion: 0.18
Nodes (11): LogFileHeader, board_name, boot_timestamp_us, circular_head_sector, limit_policy, magic, max_file_size_bytes, registered_packets_mask (+3 more)

### Community 68 - "m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.22
Nodes (3): app_main(), display_3d_image(), init_3d_matrix_struct()

### Community 70 - "m_test_data_s"
Cohesion: 0.20
Nodes (10): m_test_data_s, A_start_col, A_start_row, B_start_col, B_start_row, C_start_col, C_start_row, k (+2 more)

### Community 71 - "Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions"
Cohesion: 0.22
Nodes (7): Heinz Guderian (RTOS & DSP Lead), OpenOCD Dual-Core Trace Capture Syntax & Sizing Sweet Spot, Python SystemView Profiling & Visualization Suite (/analyzers), Standardized RTOS Timing Analysis Rules, Dual-Core Flight Loop Steady-State Benchmarks, Sub-Slice Fragmentation vs Full-Loop Boundary Timing Analysis, Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions

### Community 72 - "Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers"
Cohesion: 0.16
Nodes (16): Layer 3 Thread-Safe Attitude Ring Buffers (att_quat, att_euler, att_timestamp), 7-State Quaternion Extended Kalman Filter Model with Gyro Bias Estimation, ESP32-S3 128-bit PIE SIMD Acceleration of Matrix-Vector Propagation and Inversion, Unit-Sphere Geomagnetic Fusion with Inclination Reference Vector, Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers, ekf_roll_pitch_task(), ekf_yaw_task(), calib_save_to_nvs() (+8 more)

### Community 74 - "FC_v01 Architectural & Codebase Context Analysis Skill"
Cohesion: 0.25
Nodes (7): Graphify Knowledge Graph Rule, Graphify Query Before Grep Rule, FC_v01 Architectural & Codebase Context Analysis Skill, Codebase Graph Traversal Workflow, Zettelkasten Note Vault & ADR Scan Workflow, Workflow: analyze-context, analyze-context Execution Pipeline

### Community 75 - "Rect"
Cohesion: 0.25
Nodes (8): Rect, areaRect, height, resizeRect, width, y, Mat::Get(), Mat::getROI()

### Community 76 - "sd_logger_task.cpp"
Cohesion: 0.09
Nodes (20): PktAttQuat, q, timestamp_us, type, PktBaroFilt, pressure, timestamp_us, type (+12 more)

### Community 77 - "Zettel 0015: BLE OTA Lifecycle Architecture, Resource Stewardship & Host Stack Decoupling"
Cohesion: 0.14
Nodes (10): Driver Isolation & Explicit Notification APIs (send_ble_*_notification), Client-Side ATT MTU Adaptation & Error Unblocking, Zettel 0015: BLE OTA Lifecycle Architecture, Resource Stewardship & Host Stack Decoupling, 1. Metadata & Scope, 2. Problem Statement & Context, 3. Dependency Mapping & Graphify Subgraph Analysis, 4. Audit Findings & Resolution, Affected Translation Units & Architectural Constraints: (+2 more)

### Community 78 - "SensorCalibrationData"
Cohesion: 0.25
Nodes (8): SensorCalibrationData, accel_bias, accel_scale, gyro_bias, is_valid, mag_bias, mag_field_ref, mag_soft_iron

### Community 80 - "Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture"
Cohesion: 0.08
Nodes (24): Adam Optimizer Monotonic Convergence & 3-Plane Projection Verification, Quadric Mean Squared Error Cost Function e(v0, W), Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent, ESP32-S3 128-Bit SIMD Inner-Loop Acceleration (Vec4f Alignment), Multivariate Matrix Calculus Gradient Derivations for v0 and W, Hard-Iron Bias and Soft-Iron Distortion Problem Formulation, 6-Parameter Symmetric Soft-Iron Matrix Formulation (9-Parameter General Ellipsoid), BLE Characteristic UUID Isolation Reference (+16 more)

### Community 81 - "ESP-DSP Examples Overview and Structure"
Cohesion: 0.18
Nodes (9): ESP-DSP IIR Filter Example Project, ESP-DSP IIR Filter Main Component, ESP-DSP IIR Filter Component Dependency Manifest, ESP-DSP IIR Filter Example Guide, ESP-DSP Extended Kalman Filter Example Project, ESP-DSP Extended Kalman Filter Main Component, ESP-DSP Extended Kalman Filter Component Dependency Manifest, ESP-DSP Extended Kalman Filter Example Guide (+1 more)

### Community 82 - "ESP-DSP FIR Filter Example Guide"
Cohesion: 0.29
Nodes (6): ESP-DSP FIR Filter Example Project, ESP-DSP FIR Filter Main Component, ESP-DSP FIR Filter Component Dependency Manifest, ESP-DSP FIR Filter Example Guide, Windowed-Sinc Finite Impulse Response (FIR) Filter with Decimation (dsps_fird_f32), ESP-DSP Finite Impulse Response (FIR) Filter

### Community 83 - "accel_gyro_daq_task"
Cohesion: 0.33
Nodes (4): 500 Hz Core IMU Loop Frequency Rule, Mandatory TaskIDs Enum Enumeration, accel_gyro_daq_task(), get_all_data

### Community 84 - "HAL_SPI"
Cohesion: 0.18
Nodes (10): HAL_SPI, get_free_bytes, get_total_bytes, init, is_card_inserted, is_fatfs_mounted, mount_fatfs, transfer (+2 more)

### Community 85 - "5-Layer Decoupled Firmware Architecture"
Cohesion: 0.33
Nodes (5): 5-Layer Decoupled Firmware Architecture, Layer 1: Platform HAL & Hardware Target Drivers, Layer 2: Sensor & Peripheral Drivers Layer, Layer 4: Flight Core & Algorithms Layer, Layer 5: Application Layer

### Community 86 - "dspm_mult_ex_f32_aexx_functionality_in_cycle"
Cohesion: 0.33
Nodes (3): System Life Cycle Document (Placeholder), dspm_mult_ex_f32_aexx_functionality_in_cycle(), C_sub_check()

### Community 87 - "CPU Logo Converted 3D Matrix Image"
Cohesion: 0.33
Nodes (6): CPU Logo Converted 3D Matrix Image, Monochrome CPU Microcontroller Dot-Matrix Representation, CPU Logo Inverted Color Monochromatic Preview (64x64), Inverted Monochromatic Dot-Matrix Display Buffer, 2D Chip Graphic Input for 3D Matrix Transformation, CPU Microcontroller 2D Logo Source Image (PNG)

### Community 89 - "dsps_dct_f32.c"
Cohesion: 0.53
Nodes (4): dsps_dct_f32(), dsps_dct_f32_ref(), dsps_dct_inv_f32(), dsps_dct_inverce_f32_ref()

### Community 90 - "Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout"
Cohesion: 0.25
Nodes (5): Sarah Thueson (Embedded Software Architect), 5-Layer Decoupled Firmware Architecture Decision, FreeRTOS Task Scoping Restricted to Layer 4 (core/) or Layer 5 (app/), Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout, Zettelkasten Decision Notes Rules

### Community 91 - "ESP-DSP Applications Layout"
Cohesion: 0.33
Nodes (5): ESP-DSP Applications Layout, LVGL Graphics Integration for Spectrum Display, Spectrum Box Lite Component Dependencies, Dual-Channel Audio FFT Processing Flow, ESP32-S3-BOX-Lite Spectrum Demo Overview

### Community 92 - "ESP-DSP Library Component Specification"
Cohesion: 0.40
Nodes (4): Matrix Example Component Manifest, ESP-DSP Library Component Specification, Main Firmware Component Build Configuration, Firmware ESP-DSP Component Dependency

### Community 93 - "PktImuFilt"
Cohesion: 0.40
Nodes (5): PktImuFilt, accel, gyro, timestamp_us, type

### Community 94 - "PktImuRaw"
Cohesion: 0.40
Nodes (5): PktImuRaw, accel, gyro, timestamp_us, type

### Community 95 - "FlashRecordHeader"
Cohesion: 0.40
Nodes (5): FlashRecordHeader, crc32, magic, payload_len, schema_ver

### Community 97 - "M5Stack CoreS3 Kalman Demo Component Registration"
Cohesion: 0.50
Nodes (3): M5Stack CoreS3 Kalman Demo Component Registration, M5Stack CoreS3 Board Support Package Dependency, M5Stack CoreS3 Kalman Demo Component Manifest

### Community 98 - "build_all.sh"
Cohesion: 0.83
Nodes (3): build_for_targets(), die(), build_all.sh script

### Community 110 - "CalibReportMsg"
Cohesion: 0.25
Nodes (6): CalibReportMsg, current_step, progress_pct, status, target, timestamp_us

### Community 114 - "dsps_biquad_f32_ansi"
Cohesion: 0.33
Nodes (3): dsps_biquad_f32_ansi(), main(), test_iir_biquad()

### Community 136 - "dspm_mult.h"
Cohesion: 0.10
Nodes (8): dspm_mult_mxn_1xm_int8_ansi(), dspm_mult_s16_ansi(), dspm_mult_f32_ansi(), "Mat class ", "Mat class basic operations", "Mat class check solve ", "Mat class operators", "mat.cpp functionality"

### Community 137 - "Vec3"
Cohesion: 0.14
Nodes (15): 6-Parameter Axis-Aligned Ellipsoid Fitting Solver with Analytical Gradients, PktMagFilt, mag, timestamp_us, type, PktMagRaw, mag, timestamp_us (+7 more)

### Community 139 - "PowerMeasurement"
Cohesion: 0.40
Nodes (5): PowerMeasurement, current_ma, energy_mah, power_mw, voltage_mv

## Knowledge Gaps
- **430 isolated node(s):** `matrix_len`, `matrix_len`, `ekf13`, `Process`, `Init` (+425 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 780 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **48 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Mat` connect `Mat` to `kalman_filter_demo.cpp`, `m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp`, `ekf_imu13states`, `ekf_imu13states.cpp`, `dspm_mult.h`, `Rect`, `azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp`, `math`, `mat.cpp`, `test_mat_sub_f32.cpp`, `esp_dsp.h`, `ekf`?**
  _High betweenness centrality (0.102) - this node is a cross-community bridge._
- **Why does `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` connect `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` to `Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage`, `Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers`, `FC_v01 Architectural & Codebase Context Analysis Skill`, `FC_v01 Firmware Architecture & Structure Specification`, `BLE Telemetry 500 Hz Non-Blocking Data Flow`, `Zettel 0015: BLE OTA Lifecycle Architecture, Resource Stewardship & Host Stack Decoupling`, `app.cpp`, `Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision`, `Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture`, `main.cpp`, `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision`, `Zettel 0030: ESP32-S3 Hardware RMT PPM Receiver Decoding (Flysky FS-iA6B), Layer 1-4 Decoupled Architecture & BLE Real-Time Telemetry Pipeline`?**
  _High betweenness centrality (0.046) - this node is a cross-community bridge._
- **Why does `Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers` connect `Zettel 0032: Dual-Core 7-State Quaternion Extended Kalman Filter (EKF), SIMD Hardware Acceleration, Unit-Sphere Geomagnetic Fusion, and Inter-Task Attitude Ring Buffers` to `Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions`, `Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage`, `Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision`, `Zettel 0008: Layer 4 LPF Filter & Calibration Task Architecture`, `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision`, `Zettel 0030: ESP32-S3 Hardware RMT PPM Receiver Decoding (Flysky FS-iA6B), Layer 1-4 Decoupled Architecture & BLE Real-Time Telemetry Pipeline`, `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout`?**
  _High betweenness centrality (0.041) - this node is a cross-community bridge._
- **What connects `matrix_len`, `matrix_len`, `ekf13` to the rest of the system?**
  _430 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `dsps_fft2r_sc16_ansi.c` be split into smaller, more focused modules?**
  _Cohesion score 0.05608322026232474 - nodes in this community are weakly interconnected._
- **Should `WS2812` be split into smaller, more focused modules?**
  _Cohesion score 0.052597402597402594 - nodes in this community are weakly interconnected._
- **Should `Mat` be split into smaller, more focused modules?**
  _Cohesion score 0.05117845117845118 - nodes in this community are weakly interconnected._