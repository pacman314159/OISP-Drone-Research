# Graph Report - FC_v01  (2026-10-10)

## Corpus Check
- 420 files · ~462,372 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 127 file(s) not represented in the graph (top: .S 92, .defaults 15, (none) 11)

## Summary
- 1837 nodes · 3824 edges · 183 communities (83 shown, 100 thin omitted)
- Extraction: 89% EXTRACTED · 11% INFERRED · 0% AMBIGUOUS · INFERRED: 409 edges (avg confidence: 0.86)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4984f9f4`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- esp_log.h
- config.h
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
- sd_logger_task
- sdkconfig.h
- Attitude Outer Loop Task (100Hz)
- kalman_filter_demo.cpp
- dsps_resampler_ph.c
- dsps_fft2r_sc16_ansi.c
- B. Sensor Packet Payloads (Little-Endian, `<`)
- dsps_cplx_gen_init.c
- WS2812
- T
- init_display.c
- How to use example
- cstdint
- ekf_imu13states.cpp
- Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout
- Rect
- string
- dsps_biquad_gen_f32.c
- ekf.cpp
- math
- dsps_view
- CPU Logo Converted 3D Matrix Image
- ekf_imu13states
- test_fft2r.c
- How to use example
- ipc.h
- dsps_fft2r_fc32_ansi.c
- Zettel 0014: MicroSD Flight Logging Temporary Deprecation, FatFS Metadata Analysis & Future Architecture Roadmap
- How to use example
- m_test_data_s
- dsp_err.h
- dsps_fft4r_fc32_ansi.c
- ESP32_SPI
- 5-Layer Decoupled Architecture
- How to use example
- How to use example
- How to use example
- test_mmult.c
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
- Vec3
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
- 2. Context & Root Cause Resolutions
- main.cpp
- BleAttQuatPacket
- BMP180
- test_dsps_conv_f32_ansi.c
- Tustin Bilinear 1st-Order Low-Pass Filter
- PktBaroFilt
- RingBuffer
- 3d_kalman_demo.cpp
- PktImuRaw
- graphics_support.cpp
- PktBaroRaw
- HAL_SPI
- MPU6050::get_all_data_async
- ble_driver.cpp
- LogFileHeader
- 2. Core Architectural Decisions
- dsps_corr_f32_ansi.c
- test_dsps_memcpy_memset.c
- main/main.c
- dsps_dct_f32.c
- azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp
- NimBLECharacteristic
- PktImuFilt
- m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp
- PktAttEuler
- dspm_mult_mxn_1xm_int8_ansi.c
- dspm_mult_s16_ansi.c
- dsps_fird_init_f32
- PktMagFilt
- dsps_fird_s16_ansi.c
- dsps_fird_f32_ansi.c
- dsps_addc_f32_ansi.c
- PktAttQuat
- C_sub_check
- dspm_addc_f32_ansi.c
- dspm_sub_f32_ansi.c
- PktMagRaw
- dspm_add_f32_ansi.c
- test_mat_f32.cpp
- dspm_mulc_f32_ansi.c

## God Nodes (most connected - your core abstractions)
1. `Mat` - 98 edges
2. `MPU6050` - 31 edges
3. `ekf` - 26 edges
4. `Vec3` - 25 edges
5. `BMP180` - 23 edges
6. `ekf_imu13states` - 21 edges
7. `sd_logger_task()` - 21 edges
8. `HMC5883L` - 21 edges
9. `SDCardDriver` - 21 edges
10. `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` - 20 edges

## Surprising Connections (you probably didn't know these)
- `EKF Process & UpdateRefMeasurement Recursion` --semantically_similar_to--> `EKF Roll & Pitch Task (100Hz)`  [INFERRED] [semantically similar]
  managed_components/espressif__esp-dsp/modules/kalman/ekf_imu13states/docs/README.md → documents/RTOS Architecture.pdf
- `3. Impact & Architectural Constraints` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0013-sdCardSpiStorageAndFlightLoggingArchitecture.md → src/core/blackbox/sd_logger_task.cpp
- `3. Immediate Architectural Decision: Feature Deactivation` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0014-sdCardLoggingTemporaryDeprecationAndFindings.md → src/core/blackbox/sd_logger_task.cpp
- `4. Restoration & Hardening Roadmap (For Future Implementation)` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0014-sdCardLoggingTemporaryDeprecationAndFindings.md → src/core/blackbox/sd_logger_task.cpp
- `C. Boot Memory Protection (Zero Stack Waste on Absent Card)` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0018-sdCardLoggingRestorationAndUserButtonShutdown.md → src/core/blackbox/sd_logger_task.cpp

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

## Communities (183 total, 100 thin omitted)

### Community 1 - "config.h"
Cohesion: 0.19
Nodes (8): accel_gyro_lpf_task(), mag_lpf_task(), LowPassFilter1stOrder, initialized, x_prev, y_prev, lpf_apply(), lpf_init()

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
Cohesion: 0.08
Nodes (22): MPU6050, _accel_raw_x, _accel_raw_y, _accel_raw_z, _accel_sens, _dev_addr, enable_bypass, enable_sleep (+14 more)

### Community 6 - "Image3DMatrix"
Cohesion: 0.05
Nodes (9): convert_img(), create_matrix(), fromat_comas_spaces(), Image3DMatrix, main(), ouptput_files_gen(), BleOtaUploader, find_firmware_binary() (+1 more)

### Community 7 - "ESP32_I2C_ASYNC"
Cohesion: 0.09
Nodes (21): ESP32_I2C_ASYNC, ESP32_I2C_ASYNC::ESP32_I2C_ASYNC(), _frequency_hz, init, _initialized, _port, read_reg_async, read_reg_multi_async (+13 more)

### Community 8 - "fsm.cpp"
Cohesion: 0.10
Nodes (37): 1. Metadata & Scope, 2. Problem Statement & Architecture Requirements, 3. Core Architectural Decisions, 4. Remaining Work & Future Implementation Roadmap, 5. Traceability & Affected Files, Rule 1: Layer 5 Application Domain Scoping (`src/app/fsm.h`, `src/app/fsm.cpp`), Rule 2: Bitmask Events (`EVT_` Prefix) & Deterministic Transit Rules, Rule 3: Active Calibration State (`CALIB`) Invocation (+29 more)

### Community 9 - "test_mat_sub_f32.cpp"
Cohesion: 0.20
Nodes (26): Copy, Get, getROI, inverse, pinv, test_assert_check_area_mat_mat(), test_assert_equal_mat_mat(), "Mat class matrix subset" (+18 more)

### Community 10 - "app.cpp"
Cohesion: 0.09
Nodes (6): Rule 2: Layer 4 Task Identity & Zettel 0006 Rule 3 Preservation, app_suspend_all_tasks(), ble_ota_task(), set_ble_ota_active(), btn_hold_timer_callback(), user_btn_isr()

### Community 11 - "ekf"
Cohesion: 0.10
Nodes (21): ekf::RungeKutta(), ekf, CovariancePrediction, dFdq, dFdq_inv, eul2rotm, HP, Init (+13 more)

### Community 12 - "HMC5883L"
Cohesion: 0.09
Nodes (19): 1. Prime Directive: WS2812 is an External Hardware Peripheral, 2. Mandatory Mutex Encapsulation Rule, Zettel 0012: WS2812 External Hardware Treatment & Mutex Encapsulation Rule, HMC5883L, _dev_addr, get_all_data, get_all_data_raw, HMC5883L::HMC5883L() (+11 more)

### Community 13 - "sd_logger_task"
Cohesion: 0.14
Nodes (19): append_packet(), sd_logger_task(), SDCardDriver, _bytes_written, _cd_pin, close, _cs_pin, _file (+11 more)

### Community 14 - "sdkconfig.h"
Cohesion: 0.07
Nodes (7): dspi_conv_f32_ansi(), dsps_fft2r_rev_tables_init_fc32(), dsps_bit_rev_lookup_fc32_ae32(), dsps_fft2r_deinit_fc32(), dsps_fft4r_rev_tables_init_fc32(), dsps_cplx2real_fc32_ae32_(), dsps_fft4r_deinit_fc32()

### Community 15 - "Attitude Outer Loop Task (100Hz)"
Cohesion: 0.12
Nodes (19): Accel Gyro Calib & Filter Task (250Hz), Accel Gyro DAQ Task (500Hz), Altitude & Vz Estimator Task (100Hz), Attitude Inner Loop Task (250Hz, 4 Pri), Attitude Outer Loop Task (100Hz), Barometer Calib & Filter Task, Barometer DAQ Task, EKF Roll & Pitch Task (100Hz) (+11 more)

### Community 16 - "kalman_filter_demo.cpp"
Cohesion: 0.16
Nodes (14): app_fbm320_init(), app_mag3110_init(), app_main(), app_ssd1306_init(), display_3d_image(), draw_3d_image(), get_initial_pressure(), get_pressure_task() (+6 more)

### Community 18 - "dsps_fft2r_sc16_ansi.c"
Cohesion: 0.18
Nodes (13): microphone_read_task(), dsps_bit_rev_sc16_ansi(), dsps_cplx2real_sc16_ansi(), dsps_cplx2reC_sc16(), dsps_fft2r_init_sc16(), dsps_fft2r_sc16_ansi_(), dsps_gen_w_r2_sc16(), xtfixed_bf_1() (+5 more)

### Community 20 - "B. Sensor Packet Payloads (Little-Endian, `<`)"
Cohesion: 0.17
Nodes (12): 1. `PKT_TYPE_IMU_RAW` (`0x01`) — 29 Bytes Total, 2. `PKT_TYPE_IMU_FILT` (`0x02`) — 29 Bytes Total, 3. MicroSD Binary Log Format Specification (`LOG_XXXX.BIN`), 3. `PKT_TYPE_MAG_RAW` (`0x03`) — 17 Bytes Total, 4. `PKT_TYPE_MAG_FILT` (`0x04`) — 17 Bytes Total, 5. `PKT_TYPE_BARO_RAW` (`0x05`) — 9 Bytes Total, 6. `PKT_TYPE_BARO_FILT` (`0x06`) — 9 Bytes Total, 7. `PKT_TYPE_ATT_QUAT` (`0x07`) — 21 Bytes Total (+4 more)

### Community 21 - "dsps_cplx_gen_init.c"
Cohesion: 0.16
Nodes (9): dsps_cplx_gen_ansi(), cplx_gen_free(), dsps_cplx_gen_freq_get(), dsps_cplx_gen_freq_set(), dsps_cplx_gen_init(), dsps_cplx_gen_phase_get(), dsps_cplx_gen_phase_set(), dsps_cplx_gen_set() (+1 more)

### Community 22 - "WS2812"
Cohesion: 0.07
Nodes (29): WS2812, brightness, current_b, current_g, current_r, encode_and_transmit, init, is_on (+21 more)

### Community 23 - "T"
Cohesion: 0.38
Nodes (3): inverse3x3(), Matrix, operator+()

### Community 24 - "init_display.c"
Cohesion: 0.21
Nodes (9): bsp_display_lcd_init(), bsp_enable_feature(), bsp_i2c_init(), bsp_spi_init(), display_init(), dsp_display_brightness_init(), dsp_display_new(), dsp_display_start_with_config() (+1 more)

### Community 25 - "How to use example"
Cohesion: 0.29
Nodes (6): Basic Math Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 26 - "cstdint"
Cohesion: 0.08
Nodes (13): norm(), normalize(), HAL_I2C_ASYNC, init, read_reg_async, read_reg_multi_async, write_reg_async, write_reg_multi_async (+5 more)

### Community 27 - "ekf_imu13states.cpp"
Cohesion: 0.19
Nodes (12): app_main(), ekf::CovariancePrediction(), ekf_imu13states::Init(), ekf_imu13states::LinearizeFG(), ekf_imu13states::StateXdot(), ekf_imu13states::TestFull(), ekf_imu13states::UpdateRefMeasurement(), ekf_imu13states::UpdateRefMeasurementMagn() (+4 more)

### Community 28 - "Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout"
Cohesion: 0.06
Nodes (55): FC_v01 Firmware Architecture Specification, Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout, Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision, Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision, Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule, Zettel 0004: Multi-Type SIMD Acceleration & ESP-DSP Build Integration, Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing, Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions (+47 more)

### Community 29 - "Rect"
Cohesion: 0.14
Nodes (15): app_main(), Rect, areaRect, height, resizeRect, width, x, y (+7 more)

### Community 31 - "dsps_biquad_gen_f32.c"
Cohesion: 0.26
Nodes (12): audio_process_task(), buttons_process_task(), dsps_biquad_gen_allpass180_f32(), dsps_biquad_gen_allpass360_f32(), dsps_biquad_gen_bpf0db_f32(), dsps_biquad_gen_bpf_f32(), dsps_biquad_gen_highShelf_f32(), dsps_biquad_gen_hpf_f32() (+4 more)

### Community 32 - "ekf.cpp"
Cohesion: 0.11
Nodes (18): ekf::dFdq(), ekf::dFdq_inv(), ekf::eul2rotm(), ekf::qProduct(), ekf::quat2eul(), ekf::quat2rotm(), ekf::rotm2eul(), ekf::rotm2quat() (+10 more)

### Community 33 - "math"
Cohesion: 0.11
Nodes (8): app_main(), process_and_show(), generate_FIR_coefficients(), dsps_wind_blackman_f32(), dsps_wind_blackman_harris_f32(), dsps_wind_blackman_nuttall_f32(), dsps_wind_flat_top_f32(), dsps_wind_nuttall_f32()

### Community 34 - "dsps_view"
Cohesion: 0.16
Nodes (11): app_main(), process_and_show(), app_main(), app_main(), ShowIIRfilter(), dsps_d_gen_f32(), dsps_tone_gen_f32(), dsps_view() (+3 more)

### Community 35 - "CPU Logo Converted 3D Matrix Image"
Cohesion: 0.33
Nodes (6): CPU Logo Converted 3D Matrix Image, Monochrome CPU Microcontroller Dot-Matrix Representation, CPU Logo Inverted Color Monochromatic Preview (64x64), Inverted Monochromatic Dot-Matrix Display Buffer, 2D Chip Graphic Input for 3D Matrix Transformation, CPU Microcontroller 2D Logo Source Image (PNG)

### Community 36 - "ekf_imu13states"
Cohesion: 0.11
Nodes (17): image_3d_matrix_kalman_s, ekf13, matrix_len, image_3d_matrix_s, matrix_len, ekf_imu13states::Test(), ekf_imu13states, accel0 (+9 more)

### Community 38 - "How to use example"
Cohesion: 0.29
Nodes (6): 2D convolution Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 40 - "ipc.h"
Cohesion: 0.14
Nodes (10): is_ble_ota_active(), set_ble_connected(), FcBleServerCallbacks, onConnect, onDisconnect, p_led, OtaDataChunk, data (+2 more)

### Community 41 - "dsps_fft2r_fc32_ansi.c"
Cohesion: 0.34
Nodes (12): app_main(), dsp_is_power_of_two(), dsps_bit_rev2r_fc32(), dsps_bit_rev_fc32_ansi(), dsps_bit_rev_lookup_fc32_ansi(), dsps_cplx2reC_fc32_ansi(), dsps_fft2r_fc32_ansi_(), dsps_fft2r_init_fc32() (+4 more)

### Community 42 - "Zettel 0014: MicroSD Flight Logging Temporary Deprecation, FatFS Metadata Analysis & Future Architecture Roadmap"
Cohesion: 0.25
Nodes (8): 1. Metadata & Scope, 2. Context & Root Cause Analysis, 3. Immediate Architectural Decision: Feature Deactivation, 4. Restoration & Hardening Roadmap (For Future Implementation), 5. Traceability & Cross-References, A. The 0-Byte Empty Log File Phenomenon (`LOG_XXXX.BIN`), B. Unhandled Mount Failure & System Panic (`Guru Meditation Error: LoadProhibited`), Zettel 0014: MicroSD Flight Logging Temporary Deprecation, FatFS Metadata Analysis & Future Architecture Roadmap

### Community 43 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Dot Product Calculation Example, Example output, Hardware required, How to use example

### Community 44 - "m_test_data_s"
Cohesion: 0.20
Nodes (10): m_test_data_s, A_start_col, A_start_row, B_start_col, B_start_row, C_start_col, C_start_row, k (+2 more)

### Community 45 - "dsp_err.h"
Cohesion: 0.07
Nodes (13): "dspm_add_f32_ae32 benchmark", "dspm_add_f32_ae32 functionality", "dspm_add_f32_ansi benchmark", "dspm_add_f32_ansi functionality", "dspm_addc_f32_ansi benchmark", "dspm_mult_ex_f32_aexx benchmark", "dspm_mult_ex_f32_aexx functionality", "dspm_mult_ex_f32_ansi benchmark" (+5 more)

### Community 46 - "dsps_fft4r_fc32_ansi.c"
Cohesion: 0.30
Nodes (9): app_main(), show_FFT(), dsp_power_of_two(), dsps_bit_rev4r_direct_fc32_ansi(), dsps_bit_rev4r_fc32(), dsps_cplx2real_fc32_ansi_(), dsps_fft4r_fc32_ansi_(), dsps_fft4r_init_fc32() (+1 more)

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

### Community 52 - "test_mmult.c"
Cohesion: 0.22
Nodes (4): dsps_sqrt_f32_ansi(), dsps_sqrtf_f32_ansi(), main(), test_mmult()

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
Cohesion: 0.11
Nodes (4): tie_log(), dsps_dctiv_f32(), dsps_dstiv_f32(), dsps_firmr_s16_ansi()

### Community 82 - "Vec3"
Cohesion: 0.10
Nodes (19): Vec3, BleAccelPacket, accel, timestamp, BleAttEulerPacket, euler, timestamp, BleBaroPacket (+11 more)

### Community 83 - "audio_amp_main.c"
Cohesion: 0.27
Nodes (5): audio_read_task(), convert_float2short(), convert_short2float(), digitalLimiter(), dsps_mulc_f32_ansi()

### Community 125 - "2. Context & Root Cause Resolutions"
Cohesion: 0.14
Nodes (12): 1. Metadata & Scope, 2. Context & Root Cause Resolutions, 4. Traceability & Affected Files, A. FatFS Directory Entry POSIX Synchronization (`fsync`), B. Circular Overwrite Pointer & Odometer Reset, C. Boot Memory Protection (Zero Stack Waste on Absent Card), D. Periodic 1.0 Hz Intermediate Metadata Sync Timer, E. Hardware USER Button 3-Second Hold & Graceful Shutdown (+4 more)

### Community 127 - "main.cpp"
Cohesion: 0.15
Nodes (8): app_start(), fsm_init(), user_button_init(), ble_driver_init(), app_main(), init_ipc(), sysview_init_markers(), init

### Community 128 - "BleAttQuatPacket"
Cohesion: 0.50
Nodes (3): BleAttQuatPacket, q, timestamp

### Community 130 - "BMP180"
Cohesion: 0.06
Nodes (41): 1. Metadata & Context, 2. Core Architectural Decisions, 4. Cross-References & Traceability, A. Non-Blocking Two-Phase I2C Transaction Model, B. Pipelined Inter-Loop Conversion (Zero Intra-Loop Yield), C. Oversampling Setting (OSS) & Timing Feasibility, D. Interleaved Low-Frequency Temperature Refresh, E. Elimination of Legacy Driver Deficiencies (+33 more)

### Community 131 - "test_dsps_conv_f32_ansi.c"
Cohesion: 0.18
Nodes (3): dsps_ccorr_f32_ansi(), dsps_conv_f32_ansi(), dsps_conv_f32_ref()

### Community 139 - "PktBaroFilt"
Cohesion: 0.50
Nodes (4): PktBaroFilt, pressure, timestamp_us, type

### Community 140 - "RingBuffer"
Cohesion: 0.21
Nodes (4): Rule 3: Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (Layer 4), RingBuffer, _buffer, _head

### Community 141 - "3d_kalman_demo.cpp"
Cohesion: 0.30
Nodes (11): app_init(), app_main(), display_3d_image(), draw_3d_image_task(), init_3d_matrix_struct(), read_bmi270_data(), read_bmi270_reg(), read_bmm150_data() (+3 more)

### Community 142 - "PktImuRaw"
Cohesion: 0.40
Nodes (5): PktImuRaw, accel, gyro, timestamp_us, type

### Community 143 - "graphics_support.cpp"
Cohesion: 0.25
Nodes (10): dispaly_esp_text(), display_3d_image(), draw_3d_image_task(), init_perspective_matrix(), print_matrix(), update_rotation_matrix(), update_scaling_matrix(), update_translation_matrix() (+2 more)

### Community 147 - "PktBaroRaw"
Cohesion: 0.50
Nodes (4): PktBaroRaw, pressure, timestamp_us, type

### Community 151 - "HAL_SPI"
Cohesion: 0.18
Nodes (10): HAL_SPI, get_free_bytes, get_total_bytes, init, is_card_inserted, is_fatfs_mounted, mount_fatfs, transfer (+2 more)

### Community 153 - "ble_driver.cpp"
Cohesion: 0.20
Nodes (26): Strict 1:1 Task-to-Characteristic Isolation, 3. Impact & Architectural Constraints, accel_gyro_daq_task(), mag_daq_task(), ble_accel_filt_trans_task(), ble_accel_raw_trans_task(), ble_baro_raw_trans_task(), ble_gyro_filt_trans_task() (+18 more)

### Community 154 - "LogFileHeader"
Cohesion: 0.18
Nodes (11): LogFileHeader, board_name, boot_timestamp_us, circular_head_sector, limit_policy, magic, max_file_size_bytes, registered_packets_mask (+3 more)

### Community 155 - "2. Core Architectural Decisions"
Cohesion: 0.25
Nodes (8): 1. Metadata & Context, 2. Core Architectural Decisions, 3. Impact & Architectural Constraints, Rule 1: Unified `HAL_SPI` Abstraction & Shared Bus Decoupling (Layer 1), Rule 2: Peripheral Driver Object Ownership (`SDCardDriver`, Layer 2), Rule 4: 512-Byte Sector Accumulator & Dual-Trigger Flush, Rule 5: Pure 4-Digit Incremental File Scheme & Self-Describing Binary Header, Zettel 0013: Unified HAL_SPI Abstraction, 100 Hz Snapshot Logging & Multi-Rate MicroSD Storage

### Community 160 - "main/main.c"
Cohesion: 0.43
Nodes (4): convert_to_rgb(), image_display_task(), spectrum2d_picture(), spectrum2d_picture_init()

### Community 161 - "dsps_dct_f32.c"
Cohesion: 0.53
Nodes (4): dsps_dct_f32(), dsps_dct_f32_ref(), dsps_dct_inv_f32(), dsps_dct_inverce_f32_ref()

### Community 162 - "azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.27
Nodes (3): app_main(), app_ssd1306_init(), init_3d_matrix_struct()

### Community 164 - "PktImuFilt"
Cohesion: 0.40
Nodes (5): PktImuFilt, accel, gyro, timestamp_us, type

### Community 166 - "PktAttEuler"
Cohesion: 0.50
Nodes (4): PktAttEuler, euler, timestamp_us, type

### Community 170 - "PktMagFilt"
Cohesion: 0.50
Nodes (4): PktMagFilt, mag, timestamp_us, type

### Community 174 - "PktAttQuat"
Cohesion: 0.50
Nodes (4): PktAttQuat, q, timestamp_us, type

### Community 181 - "PktMagRaw"
Cohesion: 0.50
Nodes (4): PktMagRaw, mag, timestamp_us, type

### Community 184 - "test_mat_f32.cpp"
Cohesion: 0.19
Nodes (6): dspm_mult_f32_ansi(), "Mat class ", "Mat class basic operations", "Mat class check solve ", "Mat class operators", "mat.cpp functionality"

## Knowledge Gaps
- **372 isolated node(s):** `matrix_len`, `matrix_len`, `ekf13`, `Process`, `Init` (+367 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 685 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **100 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Mat` connect `Mat` to `ekf.cpp`, `ekf_imu13states`, `esp_dsp.h`, `test_mat_sub_f32.cpp`, `ekf`, `3d_kalman_demo.cpp`, `graphics_support.cpp`, `kalman_filter_demo.cpp`, `test_mat_f32.cpp`, `cstdint`, `ekf_imu13states.cpp`, `Rect`?**
  _High betweenness centrality (0.065) - this node is a cross-community bridge._
- **Why does `HAL_SPI` connect `HAL_SPI` to `app.cpp`, `sd_logger_task`, `ESP32_SPI`, `cstdint`, `2. Core Architectural Decisions`?**
  _High betweenness centrality (0.034) - this node is a cross-community bridge._
- **Why does `LogFileHeader` connect `LogFileHeader` to `cstdint`, `2. Core Architectural Decisions`, `B. Sensor Packet Payloads (Little-Endian, `<`)`, `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout`?**
  _High betweenness centrality (0.030) - this node is a cross-community bridge._
- **What connects `matrix_len`, `matrix_len`, `ekf13` to the rest of the system?**
  _372 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Mat` be split into smaller, more focused modules?**
  _Cohesion score 0.04644808743169399 - nodes in this community are weakly interconnected._
- **Should `dsps_fird_init_s16.c` be split into smaller, more focused modules?**
  _Cohesion score 0.06072874493927125 - nodes in this community are weakly interconnected._
- **Should `ESP-DSP Library Overview` be split into smaller, more focused modules?**
  _Cohesion score 0.054078014184397165 - nodes in this community are weakly interconnected._