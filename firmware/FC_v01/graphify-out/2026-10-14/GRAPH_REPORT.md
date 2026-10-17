# Graph Report - FC_v01  (2026-10-14)

## Corpus Check
- 438 files · ~477,196 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 127 file(s) not represented in the graph (top: .S 92, .defaults 15, (none) 11)

## Summary
- 2013 nodes · 4138 edges · 188 communities (89 shown, 99 thin omitted)
- Extraction: 90% EXTRACTED · 10% INFERRED · 0% AMBIGUOUS · INFERRED: 434 edges (avg confidence: 0.86)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4e1b5e89`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent
- 4. Core Architectural Decisions
- Mat
- dsps_fird_init_s16.c
- ESP-DSP Library Overview
- MPU6050
- Image3DMatrix
- ESP32_I2C_ASYNC
- fsm.cpp
- test_mat_sub_f32.cpp
- app.cpp
- ekf
- HMC5883L
- SDCardDriver
- sdkconfig.h
- Attitude Outer Loop Task (100Hz)
- kalman_filter_demo.cpp
- HAL_Flash
- dsps_fft2r_sc16_ansi.c
- SYSVIEW_END
- ekf.cpp
- WS2812
- Matrix
- init_display.c
- How to use example
- config.h
- ekf_imu13states.cpp
- Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout
- Rect
- string
- dsps_biquad_gen_f32.c
- test_mmult_ex_f32_aexx.cpp
- dsp_err.h
- dsps_view
- CPU Logo Converted 3D Matrix Image
- LowPassFilter1stOrder
- Bmp180Calib
- How to use example
- onConnect
- 2. Core Architectural Decisions
- ekf_imu13states
- How to use example
- ipc.h
- test_mat_f32.cpp
- bmp180.cpp
- ESP32_SPI
- 5-Layer Decoupled Architecture
- How to use example
- How to use example
- How to use example
- 2. Core Architectural Decisions
- How to use the example
- ESP-DSP Library Component Specification
- dsps_h_gen
- M5Stack CoreS3 Kalman Demo Component Registration
- How to use example
- build_all.sh
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
- How to use example
- Standardized RTOS Timing Analysis Rules
- ESP-DSP Component Registration
- Layer 1 Zero-Fallback Rule
- math
- examples/README.md
- 3. Core Architectural Decisions
- dsps_dotprod_s16_ansi.c
- dsps_dp_s8_ansi.c
- dsps_dotprod_f32_ansi.c
- dsps_dotprode_f32_ansi.c
- ble_packet.h
- 3. Core Architectural Decisions
- dsps_biquad_sf32_ansi.c
- dsps_add_s16_ansi.c
- dsps_add_s8_ansi.c
- dsps_add_f32_ansi.c
- dsps_mul_s8_ansi.c
- dsps_mul_f32_ansi.c
- dsps_mulc_s16_ansi.c
- dsps_sub_s16_ansi.c
- dsps_sub_s8_ansi.c
- dsps_sub_f32_ansi.c
- dspm_mult_ex_f32_ansi.c
- Graphify Knowledge Graph Rule
- RMT PPM RC Receiver Decoding
- I2C1 Bus Mutex Guard
- Spectrum Box Lite CMake Project Definition
- Firmware Coding Conventions & Architecture Rules
- Max Ducson (Chief Mechatronics Engineer)
- Drone Engineering & Software Architecture Team
- Unified Matrix<T, ROWS, COLS> Template Architecture
- Mandatory TaskIDs Enum Enumeration
- Dual-Core Flight Loop Steady-State Benchmarks
- ESP-DSP Changelog Document
- EKF Noise Covariance Matrices (Q and R)
- BMP180
- graphics_support.cpp
- CalibReportMsg
- test_dsps_conv_f32_ansi.c
- Tustin Bilinear 1st-Order Low-Pass Filter
- 3. Core Architectural Decisions & Specifications
- RingBuffer
- Vec3
- m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp
- azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp
- m_test_data_s
- HAL_SPI
- dsps_sqrt_f32_ansi.c
- ble_driver.cpp
- LogFileHeader
- 2. Core Architectural Decisions
- dsps_corr_f32_ansi.c
- main/main.c
- aes3_tie_log.c
- dsps_dct_f32.c
- test_iir_biquad.c
- NimBLECharacteristic
- ble_ota_task
- 3d_kalman_demo.cpp
- sd_logger_task
- PktImuFilt
- C_sub_check
- Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision
- PktImuRaw
- dsps_fird_s16_ansi.c
- dsps_fird_f32_ansi.c
- dsps_addc_f32_ansi.c
- dspm_add_f32_ansi.c
- dspm_addc_f32_ansi.c
- dspm_mulc_f32_ansi.c
- Zettel 0021: BLE Telemetry Dispatcher Consolidation, Subscription-Aware Gating & FCDatMon Stride Specification
- dspm_sub_f32_ansi.c
- audio_amp_main.c
- test_fft2r.c
- dsps_resampler_ph.c
- sd_logger_task.cpp
- dspm_mult_mxn_1xm_int8_ansi.c
- PktBaroRaw

## God Nodes (most connected - your core abstractions)
1. `Mat` - 98 edges
2. `MPU6050` - 31 edges
3. `ekf` - 26 edges
4. `Vec3` - 25 edges
5. `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` - 25 edges
6. `BMP180` - 23 edges
7. `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision` - 22 edges
8. `ekf_imu13states` - 21 edges
9. `HMC5883L` - 21 edges
10. `SDCardDriver` - 21 edges

## Surprising Connections (you probably didn't know these)
- `EKF Process & UpdateRefMeasurement Recursion` --semantically_similar_to--> `EKF Roll & Pitch Task (100Hz)`  [INFERRED] [semantically similar]
  managed_components/espressif__esp-dsp/modules/kalman/ekf_imu13states/docs/README.md → documents/RTOS Architecture.pdf
- `3. Impact & Architectural Constraints` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0013-sdCardSpiStorageAndFlightLoggingArchitecture.md → src/core/blackbox/sd_logger_task.cpp
- `Rule 5: Motion Unlatching & Inter-Orientation State Machine` --references--> `task_calib_accel()`  [INFERRED]
  documents/notes/0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture.md → src/core/sensor/calib_tasks.cpp
- `Rule 6: Task-Stack Lifecycle & Zero Static Overhead` --references--> `task_calib_accel()`  [INFERRED]
  documents/notes/0026-sixPointAccelCalibSimdEllipsoidFitFsmLedArchitecture.md → src/core/sensor/calib_tasks.cpp
- `B. Pipelined Inter-Loop Conversion (Zero Intra-Loop Yield)` --references--> `baro_daq_task()`  [INFERRED]
  documents/notes/0017-bmp180TwoStepI2cTransactionAndBaroDaqArchitecture.md → src/core/sensor/daq_tasks.cpp

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Cascaded Attitude Control Architecture** — documents_rtos_architecture_attitude_outer_loop, documents_rtos_architecture_attitude_inner_loop, documents_rtos_architecture_motor_mixer [EXTRACTED 0.95]
- **IMU Multi-Sensor Fusion Pipeline** — documents_rtos_architecture_accel_gyro_calib_filter, documents_rtos_architecture_mag_calib_filter, documents_rtos_architecture_ekf_roll_pitch, documents_rtos_architecture_ekf_yaw [EXTRACTED 0.95]
- **Sensor Data Acquisition and Ring Buffer Decoupling** — documents_rtos_architecture_accel_gyro_daq, documents_rtos_architecture_mag_daq, documents_rtos_architecture_baro_daq, documents_rtos_architecture_ring_buffer_newest_index_rule [EXTRACTED 0.95]
- **ESP-DSP Azure IoT Kit 3D Graphics Visualization Flow** — managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_3d_graphics_3d_graphics_demo_animation, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_3d_graphics_3d_graphics_matrix_rendering, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_3d_graphics_readme_app_guide [EXTRACTED 1.00]
- **ESP-DSP Azure IoT Kit Kalman Filter Visualization Flow** — managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_kalman_filter_kalman_filter_demo_animation, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_kalman_filter_kalman_filter_attitude_estimation, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_kalman_filter_readme_imu_sensor_fusion [EXTRACTED 1.00]
- **ESP-DSP Core Digital Signal Processing Primitives** — managed_components_espressif__esp_dsp_readme_matrix_operations, managed_components_espressif__esp_dsp_readme_fft, managed_components_espressif__esp_dsp_readme_iir_filter, managed_components_espressif__esp_dsp_readme_fir_filter, managed_components_espressif__esp_dsp_readme_vector_math, managed_components_espressif__esp_dsp_readme_kalman_filter [EXTRACTED 1.00]
- **5-Layer Decoupled Firmware Hierarchy** — documents_firmware_structure_layer_1_hal, documents_firmware_structure_layer_2_drivers, documents_firmware_structure_layer_3_middleware, documents_firmware_structure_layer_4_core, documents_firmware_structure_layer_5_app [EXTRACTED 1.00]
- **Dual-Microphone Real-Time Audio Visualizer Pipeline** — managed_components_espressif__esp_dsp_applications_spectrum_box_lite_readme_overview, managed_components_espressif__esp_dsp_applications_spectrum_box_lite_readme_audio_pipeline, managed_components_espressif__esp_dsp_applications_spectrum_box_lite_main_idf_component_lvgl_dep [EXTRACTED 1.00]
- **ESP-DSP 3D Matrix Transformation and Projection Pipeline** — managed_components_espressif__esp_dsp_applications_azure_board_apps_readme_mat_3d_projection, managed_components_espressif__esp_dsp_applications_azure_board_apps_graphics_img_to_3d_matrix_example_readme_img_to_3d_pipeline, managed_components_espressif__esp_dsp_applications_m5stack_core_s3_apps_3d_graphics_main_cmakelists_component [INFERRED 0.95]
- **ESP-DSP IMU Kalman Filter Sensor Fusion Demonstrations** — managed_components_espressif__esp_dsp_readme_kalman_filter, managed_components_espressif__esp_dsp_applications_azure_board_apps_apps_kalman_filter_readme_imu_sensor_fusion, managed_components_espressif__esp_dsp_applications_m5stack_core_s3_apps_kalman_filter_readme_bmi270_bmm150_fusion [INFERRED 0.95]
- **SystemView Real-Time Profiling and Anti-Choking Framework** — documents_notes_0007_systemviewtracingandrtostiminganalysisconventions_anti_choking_config, documents_notes_0007_systemviewtracingandrtostiminganalysisconventions_openocd_trace_capture_syntax, documents_notes_0007_systemviewtracingandrtostiminganalysisconventions_rtos_timing_definitions, _agents_agents_jtag_anti_choking_guardrails [INFERRED 0.95]
- **Centralized FreeRTOS Task Configuration Architecture** — documents_notes_0001_taskconfigmiddlewareisrdecision_centralized_task_config_decision, documents_notes_0002_taskenumsandhalabstractiondecision_four_level_rtos_priority, documents_notes_0006_taskcreationconfigenumsandtaskargspassingdecision_mandatory_task_ids, _agents_agents_centralized_task_config_rule [INFERRED 0.95]

## Communities (188 total, 99 thin omitted)

### Community 0 - "Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent"
Cohesion: 0.10
Nodes (20): 1. Executive Summary & Problem Context, 2.1 The Geometric Model, 2.2 Objective Function (Mean Squared Error), 2.3 Multivariate Chain Rule Framework, 2.4 Gradient with respect to Hard-Iron Offset $\mathbf{v}_0$, 2.5 Gradient with respect to Symmetric Matrix $\mathbf{W}$, 2. Mathematical Model & Derivation, 3.1 Matrices & Dimensions (+12 more)

### Community 1 - "4. Core Architectural Decisions"
Cohesion: 0.25
Nodes (8): 4. Core Architectural Decisions, Rule 1: Single Unified Telemetry Dispatcher Task (`TASK_BLE_TELEM`), Rule 2: Zero Dynamic Allocation & Static BSS Channel Instances, Rule 3: Subscription-Aware Gating (`is_ble_stream_subscribed`), Rule 4: Generic Wire-Compatible Structs with Explicit Physical SI Units, get_char_by_mask(), is_ble_stream_subscribed(), send_ble_stream_notification()

### Community 2 - "Mat"
Cohesion: 0.05
Nodes (50): Mat, abs_tol, adjoint, allocate, augment, bandSolve, block, clear (+42 more)

### Community 3 - "dsps_fird_init_s16.c"
Cohesion: 0.06
Nodes (16): dsps_16_array_rev(), dsps_fird_init_s16(), dsps_fird_s16_aexx_free(), dsps_firmr_init_s16(), dsps_fir_f32_ansi(), dsps_fir_f32_free(), dsps_fir_init_f32(), dsps_firmr_init_f32() (+8 more)

### Community 4 - "ESP-DSP Library Overview"
Cohesion: 0.05
Nodes (44): ESP-DSP GitLab Pre-Commit CI Job, ESP-DSP Pre-Commit Hook Configurations, ESP32-Azure IoT Kit 3D Graphics Demo Animation, 3D Matrix Transformation and Projection on LCD, esp-dsp-azure-board-app-3d-graphics Project, Azure 3D Graphics Main Component, Azure 3D Graphics IDF Component Manifest, Azure 3D Graphics Demo Application Guide (+36 more)

### Community 5 - "MPU6050"
Cohesion: 0.07
Nodes (23): MPU6050, _accel_raw_x, _accel_raw_y, _accel_raw_z, _accel_sens, _dev_addr, enable_bypass, enable_sleep (+15 more)

### Community 6 - "Image3DMatrix"
Cohesion: 0.05
Nodes (9): convert_img(), create_matrix(), fromat_comas_spaces(), Image3DMatrix, main(), ouptput_files_gen(), BleOtaUploader, find_firmware_binary() (+1 more)

### Community 7 - "ESP32_I2C_ASYNC"
Cohesion: 0.09
Nodes (21): ESP32_I2C_ASYNC, ESP32_I2C_ASYNC::ESP32_I2C_ASYNC(), _frequency_hz, init, _initialized, _port, read_reg_async, read_reg_multi_async (+13 more)

### Community 8 - "fsm.cpp"
Cohesion: 0.08
Nodes (38): 1. Metadata & Scope, 2. Problem Statement & Architecture Requirements, 3. Core Architectural Decisions, 4. Remaining Work & Future Implementation Roadmap, 5. Traceability & Affected Files, Rule 1: Layer 5 Application Domain Scoping (`src/app/fsm.h`, `src/app/fsm.cpp`), Rule 2: Bitmask Events (`EVT_` Prefix) & Deterministic Transit Rules, Rule 3: Active Calibration State (`CALIB`) Invocation (+30 more)

### Community 9 - "test_mat_sub_f32.cpp"
Cohesion: 0.20
Nodes (26): Copy, Get, getROI, inverse, pinv, test_assert_check_area_mat_mat(), test_assert_equal_mat_mat(), "Mat class matrix subset" (+18 more)

### Community 11 - "ekf"
Cohesion: 0.10
Nodes (21): ekf::RungeKutta(), ekf, CovariancePrediction, dFdq, dFdq_inv, eul2rotm, HP, Init (+13 more)

### Community 12 - "HMC5883L"
Cohesion: 0.09
Nodes (20): 1. Prime Directive: WS2812 is an External Hardware Peripheral, 2. Mandatory Mutex Encapsulation Rule, Zettel 0012: WS2812 External Hardware Treatment & Mutex Encapsulation Rule, mag_daq_task(), HMC5883L, _dev_addr, get_all_data, get_all_data_raw (+12 more)

### Community 13 - "SDCardDriver"
Cohesion: 0.15
Nodes (15): SDCardDriver, _bytes_written, _cd_pin, close, _cs_pin, _file, find_highest_log_id, flush (+7 more)

### Community 14 - "sdkconfig.h"
Cohesion: 0.07
Nodes (3): dspi_conv_f32_ansi(), dsps_cplx2real_fc32_ae32_(), context_switch_timer_callback()

### Community 15 - "Attitude Outer Loop Task (100Hz)"
Cohesion: 0.12
Nodes (19): Accel Gyro Calib & Filter Task (250Hz), Accel Gyro DAQ Task (500Hz), Altitude & Vz Estimator Task (100Hz), Attitude Inner Loop Task (250Hz, 4 Pri), Attitude Outer Loop Task (100Hz), Barometer Calib & Filter Task, Barometer DAQ Task, EKF Roll & Pitch Task (100Hz) (+11 more)

### Community 16 - "kalman_filter_demo.cpp"
Cohesion: 0.18
Nodes (14): app_fbm320_init(), app_mag3110_init(), app_main(), app_ssd1306_init(), display_3d_image(), draw_3d_image(), get_initial_pressure(), get_pressure_task() (+6 more)

### Community 17 - "HAL_Flash"
Cohesion: 0.09
Nodes (22): FlashStorageDriver, calculate_crc32, erase_namespace, erase_record, init, _initialized, read_record, write_record (+14 more)

### Community 18 - "dsps_fft2r_sc16_ansi.c"
Cohesion: 0.05
Nodes (48): microphone_read_task(), app_main(), app_main(), ShowIIRfilter(), dsp_is_power_of_two(), dsp_power_of_two(), dsps_bit_rev_sc16_ansi(), dsps_cplx2real_sc16_ansi() (+40 more)

### Community 20 - "SYSVIEW_END"
Cohesion: 0.17
Nodes (20): 3. Impact & Architectural Constraints, Rule 3: Task Spawning & Passive Halting Lifecycle, 1. Metadata & Scope, 2. Problem Statement & Engineering Constraints, 3. Core Architectural Decisions, 4. Cross-References & Traceability, Rule 1: Mathematical Model & Direct Unit-Sphere Mapping, Rule 2: Time-Sliced SIMD Adam Optimizer (`ellipsoid_fit.cpp`) (+12 more)

### Community 21 - "ekf.cpp"
Cohesion: 0.11
Nodes (18): ekf::dFdq(), ekf::dFdq_inv(), ekf::eul2rotm(), ekf::qProduct(), ekf::quat2eul(), ekf::quat2rotm(), ekf::rotm2eul(), ekf::rotm2quat() (+10 more)

### Community 22 - "WS2812"
Cohesion: 0.06
Nodes (35): status_led_task(), Color, b, g, r, get_palette_color(), WS2812, brightness (+27 more)

### Community 23 - "Matrix"
Cohesion: 0.26
Nodes (8): dot(), hadamard(), inverse3x3(), Matrix, norm(), norm_sq(), normalize(), operator+()

### Community 24 - "init_display.c"
Cohesion: 0.23
Nodes (8): bsp_display_lcd_init(), bsp_enable_feature(), bsp_i2c_init(), bsp_spi_init(), display_init(), dsp_display_brightness_init(), dsp_display_new(), dsp_display_start_with_config()

### Community 25 - "How to use example"
Cohesion: 0.29
Nodes (6): Basic Math Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 26 - "config.h"
Cohesion: 0.07
Nodes (11): HAL_I2C_ASYNC, init, read_reg_async, read_reg_multi_async, write_reg_async, write_reg_multi_async, HAL_I2C, read_reg (+3 more)

### Community 27 - "ekf_imu13states.cpp"
Cohesion: 0.23
Nodes (10): app_main(), ekf::CovariancePrediction(), ekf_imu13states::Init(), ekf_imu13states::LinearizeFG(), ekf_imu13states::TestFull(), ekf_imu13states::UpdateRefMeasurement(), ekf_imu13states::UpdateRefMeasurementMagn(), qProduct (+2 more)

### Community 28 - "Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout"
Cohesion: 0.09
Nodes (42): FC_v01 Firmware Architecture Specification, Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout, Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision, Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision, Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule, Zettel 0004: Multi-Type SIMD Acceleration & ESP-DSP Build Integration, Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing, Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions (+34 more)

### Community 29 - "Rect"
Cohesion: 0.14
Nodes (14): app_main(), ekf_imu13states::StateXdot(), SkewSym4x4, Rect, areaRect, height, resizeRect, width (+6 more)

### Community 30 - "string"
Cohesion: 0.12
Nodes (4): "ekf_imu13states functionality gyro and magn", "ekf_imu13states functionality gyro only", "dspm_mult_ex_f32_ansi benchmark", "dspm_mult_ex_f32_ansi functionality"

### Community 31 - "dsps_biquad_gen_f32.c"
Cohesion: 0.26
Nodes (12): audio_process_task(), buttons_process_task(), dsps_biquad_gen_allpass180_f32(), dsps_biquad_gen_allpass360_f32(), dsps_biquad_gen_bpf0db_f32(), dsps_biquad_gen_bpf_f32(), dsps_biquad_gen_highShelf_f32(), dsps_biquad_gen_hpf_f32() (+4 more)

### Community 32 - "test_mmult_ex_f32_aexx.cpp"
Cohesion: 0.10
Nodes (11): "dspm_add_f32_ae32 benchmark", "dspm_add_f32_ae32 functionality", "dspm_add_f32_ansi benchmark", "dspm_add_f32_ansi functionality", "dspm_addc_f32_ansi benchmark", "dspm_mult_ex_f32_aexx benchmark", "dspm_mult_ex_f32_aexx functionality", "dspm_mulc_f32_ansi benchmark" (+3 more)

### Community 33 - "dsp_err.h"
Cohesion: 0.06
Nodes (5): dsps_dctiv_f32(), dsps_dstiv_f32(), dsps_bit_rev_lookup_fc32_ae32(), dsps_firmr_s16_ansi(), dsps_firmr_f32_ansi()

### Community 34 - "dsps_view"
Cohesion: 0.13
Nodes (11): app_main(), process_and_show(), app_main(), app_main(), show_FFT(), dsps_fird_init_f32(), dsps_tone_gen_f32(), dsps_view() (+3 more)

### Community 35 - "CPU Logo Converted 3D Matrix Image"
Cohesion: 0.33
Nodes (6): CPU Logo Converted 3D Matrix Image, Monochrome CPU Microcontroller Dot-Matrix Representation, CPU Logo Inverted Color Monochromatic Preview (64x64), Inverted Monochromatic Dot-Matrix Display Buffer, 2D Chip Graphic Input for 3D Matrix Transformation, CPU Microcontroller 2D Logo Source Image (PNG)

### Community 36 - "LowPassFilter1stOrder"
Cohesion: 0.15
Nodes (16): Rule 1: Mathematical Model & 6-Parameter Axis-Aligned Solver, LowPassFilter1stOrder, b, initialized, x_prev, y_prev, lpf_apply(), lpf_init() (+8 more)

### Community 37 - "Bmp180Calib"
Cohesion: 0.17
Nodes (12): Bmp180Calib, ac1, ac2, ac3, ac4, ac5, ac6, b1 (+4 more)

### Community 38 - "How to use example"
Cohesion: 0.29
Nodes (6): 2D convolution Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 40 - "onConnect"
Cohesion: 0.32
Nodes (5): is_ble_ota_active(), set_ble_connected(), FcBleServerCallbacks, onConnect, onDisconnect

### Community 41 - "2. Core Architectural Decisions"
Cohesion: 0.18
Nodes (11): 1. Metadata & Context, 2. Core Architectural Decisions, 4. Cross-References & Traceability, A. Non-Blocking Two-Phase I2C Transaction Model, B. Pipelined Inter-Loop Conversion (Zero Intra-Loop Yield), C. Oversampling Setting (OSS) & Timing Feasibility, D. Interleaved Low-Frequency Temperature Refresh, E. Elimination of Legacy Driver Deficiencies (+3 more)

### Community 42 - "ekf_imu13states"
Cohesion: 0.11
Nodes (17): image_3d_matrix_kalman_s, ekf13, matrix_len, image_3d_matrix_s, matrix_len, ekf_imu13states::Test(), ekf_imu13states, accel0 (+9 more)

### Community 43 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Dot Product Calculation Example, Example output, Hardware required, How to use example

### Community 44 - "ipc.h"
Cohesion: 0.10
Nodes (15): FlashStorageDriver, LedCommand, color, pattern, OtaDataChunk, data, length, SensorCalibrationData (+7 more)

### Community 45 - "test_mat_f32.cpp"
Cohesion: 0.14
Nodes (7): dspm_mult_s16_ansi(), dspm_mult_f32_ansi(), "Mat class ", "Mat class basic operations", "Mat class check solve ", "Mat class operators", "mat.cpp functionality"

### Community 46 - "bmp180.cpp"
Cohesion: 0.27
Nodes (7): BMP180::BMP180(), calculate_b5, calculate_temperature, init, pressure_to_altitude, read_calibration_data, read_temperature_raw

### Community 47 - "ESP32_SPI"
Cohesion: 0.12
Nodes (16): ESP32_SPI, ESP32_SPI::ESP32_SPI(), get_free_bytes, get_total_bytes, init, _initialized, is_card_inserted, is_fatfs_mounted (+8 more)

### Community 48 - "5-Layer Decoupled Architecture"
Cohesion: 0.25
Nodes (7): Sarah Thueson (Embedded Software Architect), 5-Layer Decoupled Architecture, Layer 1 Platform HAL & Hardware Target Drivers Layer, Layer 2 Sensor & Peripheral Drivers Layer, Layer 3 Middleware & IPC Layer, Layer 4 Flight Core & Algorithms Layer, Layer 5 Application Layer

### Community 49 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Example output, FFT 4 Real Input Example, Hardware required, How to use example

### Community 50 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Example output, FFT Example, Hardware required, How to use example

### Community 51 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Example output, FFT Window Example, Hardware required, How to use example

### Community 52 - "2. Core Architectural Decisions"
Cohesion: 0.22
Nodes (9): 1. Metadata & Context, 2. Core Architectural Decisions, 3. Empirical Benchmarks & Hardware Observations (2026-10-06), Rule 3: NimBLE Host Task Decoupling & Stack Protection, Rule 4: Client-Side ATT MTU Adaptation & Error Unblocking, Rule 5: Environment Simplification & Local Project Compilation, Rule 6: Bluetooth 5.0 2M PHY Negotiation & DLE 251 Link Layer Acceleration, Rule 7: Windowed WRITE_NR Pipelining with SPI Flash Backpressure Flow Control (+1 more)

### Community 54 - "How to use the example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Example output, FIR Filter Example, Hardware required, How to use the example

### Community 55 - "ESP-DSP Library Component Specification"
Cohesion: 0.40
Nodes (4): Matrix Example Component Manifest, ESP-DSP Library Component Specification, Main Firmware Component Build Configuration, Firmware ESP-DSP Component Dependency

### Community 57 - "M5Stack CoreS3 Kalman Demo Component Registration"
Cohesion: 0.50
Nodes (3): M5Stack CoreS3 Kalman Demo Component Registration, M5Stack CoreS3 Board Support Package Dependency, M5Stack CoreS3 Kalman Demo Component Manifest

### Community 58 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Example output, Hardware required, How to use example, IIR Filter Example

### Community 59 - "build_all.sh"
Cohesion: 0.83
Nodes (3): build_for_targets(), die(), build_all.sh script

### Community 70 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Example Output, Extended Kalman Filter, Hardware required, How to use example

### Community 72 - "Standardized RTOS Timing Analysis Rules"
Cohesion: 0.67
Nodes (3): SystemView Tracing & RTOS Timing Conventions, Heinz Guderian (RTOS & DSP Lead), Standardized RTOS Timing Analysis Rules

### Community 73 - "ESP-DSP Component Registration"
Cohesion: 0.67
Nodes (3): FC_v01 CMake Project, ESP-DSP Multi-Type SIMD Integration, ESP-DSP Component Registration

### Community 75 - "math"
Cohesion: 0.13
Nodes (8): app_main(), process_and_show(), generate_FIR_coefficients(), dsps_wind_blackman_f32(), dsps_wind_blackman_harris_f32(), dsps_wind_blackman_nuttall_f32(), dsps_wind_flat_top_f32(), dsps_wind_nuttall_f32()

### Community 77 - "3. Core Architectural Decisions"
Cohesion: 0.20
Nodes (10): 1. Metadata & Scope, 2. Problem Statement & Engineering Constraints, 3. Core Architectural Decisions, 4. Cross-References & Traceability, Rule 2: Analytical Gradients for 6-Parameter Adam Optimizer, Rule 4: Attitude-Independent Gravity Vector Divergence & Stationary Gating, Rule 5: Motion Unlatching & Inter-Orientation State Machine, Rule 6: Task-Stack Lifecycle & Zero Static Overhead (+2 more)

### Community 82 - "ble_packet.h"
Cohesion: 0.22
Nodes (6): BleAttQuatPacket, q, timestamp, BleBaroPacket, pressure, timestamp

### Community 83 - "3. Core Architectural Decisions"
Cohesion: 0.08
Nodes (24): 1. Metadata & Scope, 2. Problem Statement & Architectural Requirements, 3. Core Architectural Decisions, 4. Traceability & Affected Files, Rule 1: Layer 1 Abstract Interface & Target Decoupling (`HAL_Flash`), Rule 2: Layer 2 Peripheral Driver (`FlashStorageDriver`), Rule 3: Layer 3 Inter-Task Mutex Synchronization (`flash_mutex`), Rule 4: Centralized IPC Calibration Struct & Memory Footprint (+16 more)

### Community 125 - "BMP180"
Cohesion: 0.17
Nodes (11): baro_daq_task(), BMP180, _b5, calculate_pressure, _calib, _dev_addr, get_conversion_delay_ms, _oss (+3 more)

### Community 128 - "graphics_support.cpp"
Cohesion: 0.29
Nodes (9): dispaly_esp_text(), display_3d_image(), draw_3d_image_task(), print_matrix(), update_rotation_matrix(), update_scaling_matrix(), update_translation_matrix(), display_3d_image() (+1 more)

### Community 130 - "CalibReportMsg"
Cohesion: 0.13
Nodes (13): 1. Metadata & Scope, 2. Problem Statement & Architecture Requirements, 3. Core Architectural Decisions, 4. Cross-References & Traceability, Rule 1: RGB LED Command Queue & Standalone Color Palette, Rule 2: Inter-Process Communication & Healthcheck Protocol, Zettel 0023: FSM-Centric Sensor Calibration Orchestration and RGB LED Status Architecture Specification, CalibReportMsg (+5 more)

### Community 131 - "test_dsps_conv_f32_ansi.c"
Cohesion: 0.18
Nodes (3): dsps_ccorr_f32_ansi(), dsps_conv_f32_ansi(), dsps_conv_f32_ref()

### Community 139 - "3. Core Architectural Decisions & Specifications"
Cohesion: 0.22
Nodes (9): 1. Metadata & Scope, 2. Problem Statement & Architecture Tensions, 3. Core Architectural Decisions & Specifications, 4. Cross-References & Traceability, Rule 1: Deprecation and Removal of Phantom Attitude Task, Rule 2: Removal of Orphan IPC Primitives (`gyro_telemetry_queue` & Scalar Globals), Rule 3: Harmonized Ring Buffer Naming (`accel`, `gyro`, `mag`), Rule 4: Unification of Barometer Pressure Ring Buffer (+1 more)

### Community 140 - "RingBuffer"
Cohesion: 0.06
Nodes (25): 1. Metadata & Scope, 1. `PKT_TYPE_IMU_RAW` (`0x01`) — 29 Bytes Total (32B with natural 4B alignment, see `[Note 0019]`), 2. `PKT_TYPE_IMU_FILT` (`0x02`) — 29 Bytes Total (32B with natural 4B alignment), 3. MicroSD Binary Log Format Specification (`LOG_XXXX.BIN`), 3. `PKT_TYPE_MAG_RAW` (`0x03`) — 17 Bytes Total (20B with natural 4B alignment), 4. `PKT_TYPE_MAG_FILT` (`0x04`) — 17 Bytes Total (20B with natural 4B alignment), 4. Traceability & Affected Files, 5. `PKT_TYPE_BARO_RAW` (`0x05`) — 9 Bytes Total (12B with natural 4B alignment) (+17 more)

### Community 141 - "Vec3"
Cohesion: 0.19
Nodes (11): Rule 3: Architectural Separation of 6-Param and 9-Param Solvers, ellipsoid_fit_6param_get_params(), ellipsoid_fit_6param_init(), ellipsoid_fit_6param_step(), ellipsoid_fit_9param_get_params(), ellipsoid_fit_9param_init(), ellipsoid_fit_9param_step(), Vec3 (+3 more)

### Community 143 - "azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.24
Nodes (4): app_main(), app_ssd1306_init(), init_3d_matrix_struct(), init_perspective_matrix()

### Community 147 - "m_test_data_s"
Cohesion: 0.20
Nodes (10): m_test_data_s, A_start_col, A_start_row, B_start_col, B_start_row, C_start_col, C_start_row, k (+2 more)

### Community 151 - "HAL_SPI"
Cohesion: 0.18
Nodes (10): HAL_SPI, get_free_bytes, get_total_bytes, init, is_card_inserted, is_fatfs_mounted, mount_fatfs, transfer (+2 more)

### Community 153 - "ble_driver.cpp"
Cohesion: 0.35
Nodes (12): Strict 1:1 Task-to-Characteristic Isolation, send_ble_accel_filt_notification(), send_ble_accel_raw_notification(), send_ble_att_euler_notification(), send_ble_att_quat_notification(), send_ble_baro_filt_notification(), send_ble_baro_raw_notification(), send_ble_gyro_filt_notification() (+4 more)

### Community 154 - "LogFileHeader"
Cohesion: 0.18
Nodes (11): LogFileHeader, board_name, boot_timestamp_us, circular_head_sector, limit_policy, magic, max_file_size_bytes, registered_packets_mask (+3 more)

### Community 155 - "2. Core Architectural Decisions"
Cohesion: 0.22
Nodes (9): 1. Metadata & Context, 2. Core Architectural Decisions, 3. Impact & Architectural Constraints, Rule 1: Unified `HAL_SPI` Abstraction & Shared Bus Decoupling (Layer 1), Rule 2: Peripheral Driver Object Ownership (`SDCardDriver`, Layer 2), Rule 3: Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (Layer 4), Rule 4: 512-Byte Sector Accumulator & Dual-Trigger Flush, Rule 5: Pure 4-Digit Incremental File Scheme & Self-Describing Binary Header (+1 more)

### Community 158 - "main/main.c"
Cohesion: 0.43
Nodes (5): app_main(), convert_to_rgb(), image_display_task(), spectrum2d_picture(), spectrum2d_picture_init()

### Community 161 - "dsps_dct_f32.c"
Cohesion: 0.53
Nodes (4): dsps_dct_f32(), dsps_dct_f32_ref(), dsps_dct_inv_f32(), dsps_dct_inverce_f32_ref()

### Community 162 - "test_iir_biquad.c"
Cohesion: 0.29
Nodes (3): dsps_biquad_f32_ansi(), main(), test_iir_biquad()

### Community 164 - "ble_ota_task"
Cohesion: 0.33
Nodes (7): Rule 1: 10-Second Boot-Up Security Window & Dynamic Memory Reclamation, Rule 2: Layer 4 Task Identity & Zettel 0006 Rule 3 Preservation, app_suspend_all_tasks(), ble_ota_task(), is_ble_connected(), send_ble_ota_response(), set_ble_ota_active()

### Community 165 - "3d_kalman_demo.cpp"
Cohesion: 0.30
Nodes (11): app_init(), app_main(), display_3d_image(), draw_3d_image_task(), init_3d_matrix_struct(), read_bmi270_data(), read_bmi270_reg(), read_bmm150_data() (+3 more)

### Community 166 - "sd_logger_task"
Cohesion: 0.18
Nodes (12): 2. Context & Root Cause Analysis, A. FatFS 0-Byte Log File Phenomenon & POSIX Synchronization (`fsync`), B. Mount Failures & Boot Memory Protection (Zero Stack Waste on Absent Card), C. Circular Overwrite Pointer & Odometer Reset, D. Hardware USER Button 3-Second Hold & Graceful Shutdown, append_packet(), ChannelTracker, has_logged (+4 more)

### Community 167 - "PktImuFilt"
Cohesion: 0.40
Nodes (5): PktImuFilt, accel, gyro, timestamp_us, type

### Community 169 - "Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision"
Cohesion: 0.67
Nodes (3): 1. Metadata & Context, 2. Core Architectural Decision, Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision

### Community 170 - "PktImuRaw"
Cohesion: 0.40
Nodes (5): PktImuRaw, accel, gyro, timestamp_us, type

### Community 177 - "Zettel 0021: BLE Telemetry Dispatcher Consolidation, Subscription-Aware Gating & FCDatMon Stride Specification"
Cohesion: 0.22
Nodes (9): 1. Metadata & Scope, 2. Problem Statement & Motivation, 3. Host Monitor Interaction Contract (`FCDatMon`), 5. Architectural Verification & Backward Compatibility, 6. Cross-References & Traceability, Duplicate Packet Structs & Missing SI Units, Multi-Task Over-Engineering & RAM Exhaustion, Unconditional Popping & BLE Stack Saturation (+1 more)

### Community 179 - "audio_amp_main.c"
Cohesion: 0.27
Nodes (5): audio_read_task(), convert_float2short(), convert_short2float(), digitalLimiter(), dsps_mulc_f32_ansi()

### Community 180 - "test_fft2r.c"
Cohesion: 0.19
Nodes (4): main(), test_fft2r(), main(), test_mmult()

### Community 183 - "sd_logger_task.cpp"
Cohesion: 0.09
Nodes (20): PktAttEuler, euler, timestamp_us, type, PktAttQuat, q, timestamp_us, type (+12 more)

### Community 187 - "PktBaroRaw"
Cohesion: 0.50
Nodes (4): PktBaroRaw, pressure, timestamp_us, type

## Knowledge Gaps
- **443 isolated node(s):** `matrix_len`, `matrix_len`, `ekf13`, `Process`, `Init` (+438 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 769 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **99 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Mat` connect `Mat` to `graphics_support.cpp`, `3d_kalman_demo.cpp`, `esp_dsp.h`, `test_mat_sub_f32.cpp`, `ekf_imu13states`, `ekf`, `test_mat_f32.cpp`, `azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp`, `kalman_filter_demo.cpp`, `ekf.cpp`, `config.h`, `ekf_imu13states.cpp`, `Rect`?**
  _High betweenness centrality (0.062) - this node is a cross-community bridge._
- **Why does `LogFileHeader` connect `LogFileHeader` to `config.h`, `2. Core Architectural Decisions`, `RingBuffer`, `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout`?**
  _High betweenness centrality (0.038) - this node is a cross-community bridge._
- **Why does `sd_logger_task()` connect `sd_logger_task` to `fsm.cpp`, `SDCardDriver`, `SYSVIEW_END`, `sd_logger_task.cpp`, `2. Core Architectural Decisions`?**
  _High betweenness centrality (0.026) - this node is a cross-community bridge._
- **What connects `matrix_len`, `matrix_len`, `ekf13` to the rest of the system?**
  _443 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Magnetometer Full Rotated Ellipsoid Calibration via SIMD-Accelerated Symmetric Gradient Descent` be split into smaller, more focused modules?**
  _Cohesion score 0.1 - nodes in this community are weakly interconnected._
- **Should `Mat` be split into smaller, more focused modules?**
  _Cohesion score 0.04505888376856119 - nodes in this community are weakly interconnected._
- **Should `dsps_fird_init_s16.c` be split into smaller, more focused modules?**
  _Cohesion score 0.06072874493927125 - nodes in this community are weakly interconnected._