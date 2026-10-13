# Graph Report - FC_v01  (2026-10-11)

## Corpus Check
- 420 files · ~461,733 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 127 file(s) not represented in the graph (top: .S 92, .defaults 15, (none) 11)

## Summary
- 1835 nodes · 3796 edges · 178 communities (80 shown, 98 thin omitted)
- Extraction: 90% EXTRACTED · 10% INFERRED · 0% AMBIGUOUS · INFERRED: 381 edges (avg confidence: 0.86)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4984f9f4`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- SYSVIEW_END
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
- dsps_resampler_ph.c
- dsps_fft2r_sc16_ansi.c
- B. Sensor Packet Payloads (Little-Endian, `<`)
- ekf.cpp
- WS2812
- Vec3
- init_display.c
- How to use example
- config.h
- ekf_imu13states.cpp
- Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout
- Rect
- string
- dsps_biquad_gen_f32.c
- dsps_sqrt_f32_ansi.c
- math
- dsps_view
- CPU Logo Converted 3D Matrix Image
- PktImuFilt
- test_fft2r.c
- How to use example
- FcBleServerCallbacks
- aes3_tie_log.c
- dsps_fird_init_f32
- How to use example
- m_test_data_s
- dsp_err.h
- test_mmult.c
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
- test_iir_biquad.c
- examples/README.md
- dsp_common.h
- dsps_dotprod_s16_ansi.c
- dsps_dp_s8_ansi.c
- dsps_dotprod_f32_ansi.c
- dsps_dotprode_f32_ansi.c
- ble_packet.h
- audio_amp_main.c
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
- PktMagFilt
- HAL_I2C
- BleTelemetryChannel
- BMP180
- test_dsps_conv_f32_ansi.c
- Tustin Bilinear 1st-Order Low-Pass Filter
- ipc.cpp
- RingBuffer
- 3d_kalman_demo.cpp
- sd_logger_task.cpp
- azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp
- PktMagRaw
- HAL_SPI
- Zettel 0021: BLE Telemetry Dispatcher Consolidation, Subscription-Aware Gating & FCDatMon Stride Specification
- ble_driver.cpp
- LogFileHeader
- 2. Core Architectural Decisions
- dsps_corr_f32_ansi.c
- OtaDataChunk
- main/main.c
- dsps_dct_f32.c
- send_ble_ota_response
- ShowIIRfilter
- m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp
- ChannelTracker
- dspm_mult_s16_ansi.c
- Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision
- dsps_fird_s16_ansi.c
- dsps_fird_f32_ansi.c
- dsps_addc_f32_ansi.c
- C_sub_check
- dspm_addc_f32_ansi.c
- dspm_sub_f32_ansi.c
- dspm_add_f32_ansi.c
- test_mat_f32.cpp
- dspm_mulc_f32_ansi.c

## God Nodes (most connected - your core abstractions)
1. `Mat` - 98 edges
2. `MPU6050` - 31 edges
3. `ekf` - 26 edges
4. `BMP180` - 23 edges
5. `ekf_imu13states` - 21 edges
6. `HMC5883L` - 21 edges
7. `SDCardDriver` - 21 edges
8. `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` - 19 edges
9. `test_assert_check_area_mat_mat()` - 18 edges
10. `sd_logger_task()` - 18 edges

## Surprising Connections (you probably didn't know these)
- `EKF Process & UpdateRefMeasurement Recursion` --semantically_similar_to--> `EKF Roll & Pitch Task (100Hz)`  [INFERRED] [semantically similar]
  managed_components/espressif__esp-dsp/modules/kalman/ekf_imu13states/docs/README.md → documents/RTOS Architecture.pdf
- `3. Impact & Architectural Constraints` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0013-sdCardSpiStorageAndFlightLoggingArchitecture.md → src/core/blackbox/sd_logger_task.cpp
- `A. FatFS 0-Byte Log File Phenomenon & POSIX Synchronization (`fsync`)` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0018-sdCardLoggingRestorationAndUserButtonShutdown.md → src/core/blackbox/sd_logger_task.cpp
- `B. Mount Failures & Boot Memory Protection (Zero Stack Waste on Absent Card)` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0018-sdCardLoggingRestorationAndUserButtonShutdown.md → src/core/blackbox/sd_logger_task.cpp
- `5. Architectural Verification & Backward Compatibility` --references--> `ble_telem_task()`  [INFERRED]
  documents/notes/0021-bleTelemetryDispatcherConsolidationAndSubscriptionGating.md → src/core/telemetry/ble_telem_tasks.cpp

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

## Communities (178 total, 98 thin omitted)

### Community 1 - "SYSVIEW_END"
Cohesion: 0.16
Nodes (16): 3. Impact & Architectural Constraints, accel_gyro_lpf_task(), mag_lpf_task(), LowPassFilter1stOrder, a, initialized, x_prev, y_prev (+8 more)

### Community 2 - "Mat"
Cohesion: 0.05
Nodes (48): Mat, abs_tol, adjoint, allocate, augment, bandSolve, block, clear (+40 more)

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
Cohesion: 0.18
Nodes (21): Rule 5: Zero-Jitter Self-Gating Transition Engine (`evaluate_state_transitions`), check_match_policy(), check_passive_sensor_calibration(), check_platform_tilt(), check_rc_controller_signal(), check_sensor_stream_health(), check_stationary_condition(), check_storage_health() (+13 more)

### Community 9 - "test_mat_sub_f32.cpp"
Cohesion: 0.20
Nodes (26): Copy, Get, getROI, inverse, pinv, test_assert_check_area_mat_mat(), test_assert_equal_mat_mat(), "Mat class matrix subset" (+18 more)

### Community 11 - "ekf"
Cohesion: 0.06
Nodes (36): image_3d_matrix_kalman_s, ekf13, matrix_len, image_3d_matrix_s, matrix_len, ekf_imu13states, accel0, Init (+28 more)

### Community 12 - "HMC5883L"
Cohesion: 0.09
Nodes (19): 1. Prime Directive: WS2812 is an External Hardware Peripheral, 2. Mandatory Mutex Encapsulation Rule, Zettel 0012: WS2812 External Hardware Treatment & Mutex Encapsulation Rule, HMC5883L, _dev_addr, get_all_data, get_all_data_raw, HMC5883L::HMC5883L() (+11 more)

### Community 13 - "SDCardDriver"
Cohesion: 0.14
Nodes (19): append_packet(), sd_logger_task(), SDCardDriver, _bytes_written, _cd_pin, close, _cs_pin, _file (+11 more)

### Community 14 - "sdkconfig.h"
Cohesion: 0.07
Nodes (7): dspi_conv_f32_ansi(), dsps_fft2r_rev_tables_init_fc32(), dsps_bit_rev_lookup_fc32_ae32(), dsps_fft2r_deinit_fc32(), dsps_fft4r_rev_tables_init_fc32(), dsps_cplx2real_fc32_ae32_(), dsps_fft4r_deinit_fc32()

### Community 15 - "Attitude Outer Loop Task (100Hz)"
Cohesion: 0.12
Nodes (19): Accel Gyro Calib & Filter Task (250Hz), Accel Gyro DAQ Task (500Hz), Altitude & Vz Estimator Task (100Hz), Attitude Inner Loop Task (250Hz, 4 Pri), Attitude Outer Loop Task (100Hz), Barometer Calib & Filter Task, Barometer DAQ Task, EKF Roll & Pitch Task (100Hz) (+11 more)

### Community 16 - "kalman_filter_demo.cpp"
Cohesion: 0.15
Nodes (15): app_fbm320_init(), app_mag3110_init(), app_main(), app_ssd1306_init(), display_3d_image(), draw_3d_image(), get_initial_pressure(), get_pressure_task() (+7 more)

### Community 18 - "dsps_fft2r_sc16_ansi.c"
Cohesion: 0.06
Nodes (39): microphone_read_task(), dsp_is_power_of_two(), dsp_power_of_two(), dsps_bit_rev_sc16_ansi(), dsps_cplx2real_sc16_ansi(), dsps_cplx2reC_sc16(), dsps_fft2r_init_sc16(), dsps_fft2r_sc16_ansi_() (+31 more)

### Community 20 - "B. Sensor Packet Payloads (Little-Endian, `<`)"
Cohesion: 0.10
Nodes (20): 1. Metadata & Scope, 1. `PKT_TYPE_IMU_RAW` (`0x01`) — 29 Bytes Total (32B with natural 4B alignment, see `[Note 0019]`), 2. Context & Root Cause Analysis, 2. `PKT_TYPE_IMU_FILT` (`0x02`) — 29 Bytes Total (32B with natural 4B alignment), 3. MicroSD Binary Log Format Specification (`LOG_XXXX.BIN`), 3. `PKT_TYPE_MAG_RAW` (`0x03`) — 17 Bytes Total (20B with natural 4B alignment), 4. `PKT_TYPE_MAG_FILT` (`0x04`) — 17 Bytes Total (20B with natural 4B alignment), 4. Traceability & Affected Files (+12 more)

### Community 21 - "ekf.cpp"
Cohesion: 0.10
Nodes (20): ekf::dFdq(), ekf::dFdq_inv(), ekf::eul2rotm(), ekf::qProduct(), ekf::quat2eul(), ekf::quat2rotm(), ekf::rotm2eul(), ekf::rotm2quat() (+12 more)

### Community 22 - "WS2812"
Cohesion: 0.07
Nodes (29): WS2812, brightness, current_b, current_g, current_r, encode_and_transmit, init, is_on (+21 more)

### Community 23 - "Vec3"
Cohesion: 0.21
Nodes (7): inverse3x3(), Matrix, operator+(), Vec3, BleVec3Packet, data, timestamp

### Community 24 - "init_display.c"
Cohesion: 0.23
Nodes (8): bsp_display_lcd_init(), bsp_enable_feature(), bsp_i2c_init(), bsp_spi_init(), display_init(), dsp_display_brightness_init(), dsp_display_new(), dsp_display_start_with_config()

### Community 25 - "How to use example"
Cohesion: 0.29
Nodes (6): Basic Math Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 26 - "config.h"
Cohesion: 0.10
Nodes (8): norm(), normalize(), HAL_I2C_ASYNC, init, read_reg_async, read_reg_multi_async, write_reg_async, write_reg_multi_async

### Community 27 - "ekf_imu13states.cpp"
Cohesion: 0.13
Nodes (18): app_main(), app_main(), ekf::CovariancePrediction(), ekf_imu13states::Init(), ekf_imu13states::LinearizeFG(), ekf_imu13states::StateXdot(), ekf_imu13states::Test(), ekf_imu13states::TestFull() (+10 more)

### Community 28 - "Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout"
Cohesion: 0.07
Nodes (48): FC_v01 Firmware Architecture Specification, Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout, Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision, Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision, Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule, Zettel 0004: Multi-Type SIMD Acceleration & ESP-DSP Build Integration, Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing, Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions (+40 more)

### Community 29 - "Rect"
Cohesion: 0.25
Nodes (8): Rect, areaRect, height, resizeRect, width, y, Mat::Get(), Mat::getROI()

### Community 30 - "string"
Cohesion: 0.11
Nodes (3): "ekf_imu13states functionality gyro and magn", "ekf_imu13states functionality gyro only", dspm_mult_mxn_1xm_int8_ansi()

### Community 31 - "dsps_biquad_gen_f32.c"
Cohesion: 0.26
Nodes (12): audio_process_task(), buttons_process_task(), dsps_biquad_gen_allpass180_f32(), dsps_biquad_gen_allpass360_f32(), dsps_biquad_gen_bpf0db_f32(), dsps_biquad_gen_bpf_f32(), dsps_biquad_gen_highShelf_f32(), dsps_biquad_gen_hpf_f32() (+4 more)

### Community 33 - "math"
Cohesion: 0.14
Nodes (7): app_main(), process_and_show(), dsps_wind_blackman_f32(), dsps_wind_blackman_harris_f32(), dsps_wind_blackman_nuttall_f32(), dsps_wind_flat_top_f32(), dsps_wind_nuttall_f32()

### Community 34 - "dsps_view"
Cohesion: 0.16
Nodes (13): app_main(), process_and_show(), app_main(), app_main(), app_main(), generate_FIR_coefficients(), show_FFT(), dsps_fft4r_init_fc32() (+5 more)

### Community 35 - "CPU Logo Converted 3D Matrix Image"
Cohesion: 0.33
Nodes (6): CPU Logo Converted 3D Matrix Image, Monochrome CPU Microcontroller Dot-Matrix Representation, CPU Logo Inverted Color Monochromatic Preview (64x64), Inverted Monochromatic Dot-Matrix Display Buffer, 2D Chip Graphic Input for 3D Matrix Transformation, CPU Microcontroller 2D Logo Source Image (PNG)

### Community 36 - "PktImuFilt"
Cohesion: 0.40
Nodes (5): PktImuFilt, accel, gyro, timestamp_us, type

### Community 38 - "How to use example"
Cohesion: 0.29
Nodes (6): 2D convolution Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 40 - "FcBleServerCallbacks"
Cohesion: 0.24
Nodes (7): is_ble_ota_active(), set_ble_connected(), FcBleServerCallbacks, onConnect, onDisconnect, p_led, WS2812

### Community 43 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Dot Product Calculation Example, Example output, Hardware required, How to use example

### Community 44 - "m_test_data_s"
Cohesion: 0.20
Nodes (10): m_test_data_s, A_start_col, A_start_row, B_start_col, B_start_row, C_start_col, C_start_row, k (+2 more)

### Community 45 - "dsp_err.h"
Cohesion: 0.07
Nodes (13): "dspm_add_f32_ae32 benchmark", "dspm_add_f32_ae32 functionality", "dspm_add_f32_ansi benchmark", "dspm_add_f32_ansi functionality", "dspm_addc_f32_ansi benchmark", "dspm_mult_ex_f32_aexx benchmark", "dspm_mult_ex_f32_aexx functionality", "dspm_mult_ex_f32_ansi benchmark" (+5 more)

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
Cohesion: 0.15
Nodes (14): 1. Metadata & Context, 2. Core Architectural Decisions, 3. Empirical Benchmarks & Hardware Observations (2026-10-06), Rule 1: 10-Second Boot-Up Security Window & Dynamic Memory Reclamation, Rule 2: Layer 4 Task Identity & Zettel 0006 Rule 3 Preservation, Rule 3: NimBLE Host Task Decoupling & Stack Protection, Rule 4: Client-Side ATT MTU Adaptation & Error Unblocking, Rule 5: Environment Simplification & Local Project Compilation (+6 more)

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

### Community 75 - "test_iir_biquad.c"
Cohesion: 0.29
Nodes (3): dsps_biquad_f32_ansi(), main(), test_iir_biquad()

### Community 77 - "dsp_common.h"
Cohesion: 0.06
Nodes (5): dsps_dctiv_f32(), dsps_dstiv_f32(), dsps_firmr_s16_ansi(), dsps_firmr_f32_ansi(), context_switch_timer_callback()

### Community 82 - "ble_packet.h"
Cohesion: 0.22
Nodes (6): BleAttQuatPacket, q, timestamp, BleBaroPacket, pressure, timestamp

### Community 83 - "audio_amp_main.c"
Cohesion: 0.24
Nodes (5): audio_read_task(), convert_float2short(), convert_short2float(), digitalLimiter(), dsps_mulc_f32_ansi()

### Community 125 - "PktMagFilt"
Cohesion: 0.50
Nodes (4): PktMagFilt, mag, timestamp_us, type

### Community 127 - "HAL_I2C"
Cohesion: 0.15
Nodes (12): app_start(), fsm_init(), user_button_init(), ble_driver_init(), app_main(), init_ipc(), HAL_I2C, init (+4 more)

### Community 128 - "BleTelemetryChannel"
Cohesion: 0.18
Nodes (7): BleTelemetryChannel, _last_ingested_ts, _last_send_tick, _mask_bit, _payload_buf, _payload_count, process()

### Community 130 - "BMP180"
Cohesion: 0.06
Nodes (41): 1. Metadata & Context, 2. Core Architectural Decisions, 4. Cross-References & Traceability, A. Non-Blocking Two-Phase I2C Transaction Model, B. Pipelined Inter-Loop Conversion (Zero Intra-Loop Yield), C. Oversampling Setting (OSS) & Timing Feasibility, D. Interleaved Low-Frequency Temperature Refresh, E. Elimination of Legacy Driver Deficiencies (+33 more)

### Community 131 - "test_dsps_conv_f32_ansi.c"
Cohesion: 0.20
Nodes (3): dsps_ccorr_f32_ansi(), dsps_conv_f32_ansi(), dsps_conv_f32_ref()

### Community 139 - "ipc.cpp"
Cohesion: 0.15
Nodes (11): 3. Core Architectural Decisions, Rule 1: Layer 5 Application Domain Scoping (`src/app/fsm.h`, `src/app/fsm.cpp`), Rule 2: Bitmask Events (`EVT_` Prefix) & Deterministic Transit Rules, Rule 3: Active Calibration State (`CALIB`) Invocation, Rule 4: Stationary Detection & Passive Calibration Verification (`PREFLIGHT`), Rule 6: Layer 3 Button ISR Architecture & Decoupled Downward IPC Event Flow, handle_state_postflight(), btn_hold_timer_callback() (+3 more)

### Community 140 - "RingBuffer"
Cohesion: 0.21
Nodes (4): Rule 3: Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (Layer 4), RingBuffer, _buffer, _head

### Community 141 - "3d_kalman_demo.cpp"
Cohesion: 0.30
Nodes (11): app_init(), app_main(), display_3d_image(), draw_3d_image_task(), init_3d_matrix_struct(), read_bmi270_data(), read_bmi270_reg(), read_bmm150_data() (+3 more)

### Community 142 - "sd_logger_task.cpp"
Cohesion: 0.08
Nodes (21): PktAttEuler, euler, timestamp_us, type, PktAttQuat, q, timestamp_us, type (+13 more)

### Community 143 - "azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.21
Nodes (7): app_main(), app_ssd1306_init(), dispaly_esp_text(), display_3d_image(), draw_3d_image_task(), init_3d_matrix_struct(), update_translation_matrix()

### Community 147 - "PktMagRaw"
Cohesion: 0.50
Nodes (4): PktMagRaw, mag, timestamp_us, type

### Community 151 - "HAL_SPI"
Cohesion: 0.18
Nodes (10): HAL_SPI, get_free_bytes, get_total_bytes, init, is_card_inserted, is_fatfs_mounted, mount_fatfs, transfer (+2 more)

### Community 152 - "Zettel 0021: BLE Telemetry Dispatcher Consolidation, Subscription-Aware Gating & FCDatMon Stride Specification"
Cohesion: 0.15
Nodes (13): 1. Metadata & Scope, 2. Problem Statement & Motivation, 3. Host Monitor Interaction Contract (`FCDatMon`), 4. Core Architectural Decisions, 5. Architectural Verification & Backward Compatibility, Duplicate Packet Structs & Missing SI Units, Multi-Task Over-Engineering & RAM Exhaustion, Rule 1: Single Unified Telemetry Dispatcher Task (`TASK_BLE_TELEM`) (+5 more)

### Community 153 - "ble_driver.cpp"
Cohesion: 0.24
Nodes (16): Strict 1:1 Task-to-Characteristic Isolation, get_char_by_mask(), is_ble_connected(), is_ble_stream_subscribed(), send_ble_accel_filt_notification(), send_ble_accel_raw_notification(), send_ble_att_euler_notification(), send_ble_att_quat_notification() (+8 more)

### Community 154 - "LogFileHeader"
Cohesion: 0.18
Nodes (11): LogFileHeader, board_name, boot_timestamp_us, circular_head_sector, limit_policy, magic, max_file_size_bytes, registered_packets_mask (+3 more)

### Community 155 - "2. Core Architectural Decisions"
Cohesion: 0.25
Nodes (8): 1. Metadata & Context, 2. Core Architectural Decisions, 3. Impact & Architectural Constraints, Rule 1: Unified `HAL_SPI` Abstraction & Shared Bus Decoupling (Layer 1), Rule 2: Peripheral Driver Object Ownership (`SDCardDriver`, Layer 2), Rule 4: 512-Byte Sector Accumulator & Dual-Trigger Flush, Rule 5: Pure 4-Digit Incremental File Scheme & Self-Describing Binary Header, Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage

### Community 159 - "OtaDataChunk"
Cohesion: 0.67
Nodes (3): OtaDataChunk, data, length

### Community 160 - "main/main.c"
Cohesion: 0.43
Nodes (5): app_main(), convert_to_rgb(), image_display_task(), spectrum2d_picture(), spectrum2d_picture_init()

### Community 161 - "dsps_dct_f32.c"
Cohesion: 0.53
Nodes (4): dsps_dct_f32(), dsps_dct_f32_ref(), dsps_dct_inv_f32(), dsps_dct_inverce_f32_ref()

### Community 163 - "send_ble_ota_response"
Cohesion: 0.33
Nodes (3): OtaControlCallbacks, OtaDataCallbacks, send_ble_ota_response()

### Community 164 - "ShowIIRfilter"
Cohesion: 0.40
Nodes (3): app_main(), ShowIIRfilter(), dsps_d_gen_f32()

### Community 165 - "m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.16
Nodes (7): init_perspective_matrix(), print_matrix(), update_scaling_matrix(), app_main(), display_3d_image(), draw_3d_image_task(), init_3d_matrix_struct()

### Community 166 - "ChannelTracker"
Cohesion: 0.50
Nodes (3): ChannelTracker, has_logged, last_ts

### Community 169 - "Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision"
Cohesion: 0.67
Nodes (3): 1. Metadata & Context, 2. Core Architectural Decision, Zettel 0009: Distinct BLE GATT Characteristic UUID Isolation per Telemetry & OTA Stream Decision

### Community 184 - "test_mat_f32.cpp"
Cohesion: 0.33
Nodes (5): "Mat class ", "Mat class basic operations", "Mat class check solve ", "Mat class operators", "mat.cpp functionality"

## Knowledge Gaps
- **374 isolated node(s):** `matrix_len`, `matrix_len`, `ekf13`, `Process`, `Init` (+369 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 691 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **98 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Mat` connect `Mat` to `m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp`, `esp_dsp.h`, `test_mat_sub_f32.cpp`, `ekf`, `3d_kalman_demo.cpp`, `azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp`, `kalman_filter_demo.cpp`, `ekf.cpp`, `test_mat_f32.cpp`, `config.h`, `ekf_imu13states.cpp`, `Rect`?**
  _High betweenness centrality (0.065) - this node is a cross-community bridge._
- **Why does `LogFileHeader` connect `LogFileHeader` to `config.h`, `2. Core Architectural Decisions`, `B. Sensor Packet Payloads (Little-Endian, `<`)`, `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout`?**
  _High betweenness centrality (0.030) - this node is a cross-community bridge._
- **Why does `HAL_SPI` connect `HAL_SPI` to `app.cpp`, `SDCardDriver`, `ESP32_SPI`, `config.h`, `2. Core Architectural Decisions`?**
  _High betweenness centrality (0.028) - this node is a cross-community bridge._
- **What connects `matrix_len`, `matrix_len`, `ekf13` to the rest of the system?**
  _374 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Mat` be split into smaller, more focused modules?**
  _Cohesion score 0.04644808743169399 - nodes in this community are weakly interconnected._
- **Should `dsps_fird_init_s16.c` be split into smaller, more focused modules?**
  _Cohesion score 0.06072874493927125 - nodes in this community are weakly interconnected._
- **Should `ESP-DSP Library Overview` be split into smaller, more focused modules?**
  _Cohesion score 0.054078014184397165 - nodes in this community are weakly interconnected._