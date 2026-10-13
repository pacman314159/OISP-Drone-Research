# Graph Report - FC_v01  (2026-10-05)

## Corpus Check
- 411 files · ~454,320 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 128 file(s) not represented in the graph (top: .S 92, .defaults 15, (none) 11)

## Summary
- 1690 nodes · 3537 edges · 182 communities (79 shown, 103 thin omitted)
- Extraction: 90% EXTRACTED · 10% INFERRED · 0% AMBIGUOUS · INFERRED: 364 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4984f9f4`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- string
- dsps_cplx_gen_init.c
- Mat
- dsps_fird_s16_aexx_free
- ESP-DSP Library Overview
- MPU6050
- Image3DMatrix
- ESP32_I2C_ASYNC
- app.cpp
- test_mat_sub_f32.cpp
- mat.cpp
- ekf
- HMC5883L
- SDCardDriver
- Attitude Outer Loop Task (100Hz)
- kalman_filter_demo.cpp
- esp_log.h
- dsps_fft2r_sc16_ansi.c
- dsps_math_main.c
- dsps_fft4r_fc32_ansi.c
- matrix.h
- WS2812
- Vec3
- init_display.c
- How to use example
- config.h
- ekf_imu13states.cpp
- Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout
- Rect
- audio_amp_main.c
- dsps_biquad_gen_f32.c
- ble_driver.cpp
- math
- 3d_kalman_demo.cpp
- CPU Logo Converted 3D Matrix Image
- azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp
- m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp
- How to use example
- FcBleServerCallbacks
- sysview_tracing.h
- Zettel 0014: MicroSD Flight Logging Temporary Deprecation, FatFS Metadata Analysis & Future Architecture Roadmap
- How to use example
- m_test_data_s
- dsp_err.h
- lpf.h
- ESP32_SPI
- 5-Layer Decoupled Architecture
- How to use example
- How to use example
- How to use example
- freertos
- dsps_fft2r_fc32_ansi.c
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
- dsps_resampler_ph.c
- Standardized RTOS Timing Analysis Rules
- ESP-DSP Component Registration
- Layer 1 Zero-Fallback Rule
- stdio
- examples/README.md
- dsp_common.h
- dsps_dotprod_s16_ansi.c
- dsps_dp_s8_ansi.c
- dsps_dotprod_f32_ansi.c
- dsps_dotprode_f32_ansi.c
- ChannelTracker
- dsps_sqrt_f32_ansi.c
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
- sd_logger_task.cpp
- HAL_I2C
- Tustin Bilinear 1st-Order Low-Pass Filter
- ble_telem_tasks.cpp
- RingBuffer
- stdint
- dsp_types.h
- ekf_imu13states
- C_sub_check
- HAL_SPI
- 2. Core Architectural Decisions
- LogFileHeader
- 2. Core Architectural Decisions
- dsps_corr_f32_ansi.c
- test_dsps_memcpy_memset.c
- test_dsps_conv_f32_ansi.c
- HAL_I2C_ASYNC
- graphics_support.cpp
- test_mat_f32.cpp
- dsps_view
- dsps_fir_main.c
- dsps_dct_f32.c
- dsps_cplx_gen
- dsps_fft4r.h
- PktImuFilt
- BleBaroPacket
- dsps_fird_s16_ansi.c
- dsps_fird_f32_ansi.c
- dsps_addc_f32_ansi.c
- dsps_mul_s16_ansi
- dspm_add_f32_ansi.c
- dspm_addc_f32_ansi.c
- dspm_mult_mxn_1xm_int8_ansi.c
- dspm_mult_s16_ansi.c
- dspm_mulc_f32_ansi.c
- dspm_sub_f32_ansi.c
- dsps_d_gen_f32

## God Nodes (most connected - your core abstractions)
1. `Mat` - 98 edges
2. `MPU6050` - 31 edges
3. `ekf` - 26 edges
4. `Vec3` - 25 edges
5. `ekf_imu13states` - 21 edges
6. `HMC5883L` - 21 edges
7. `SDCardDriver` - 21 edges
8. `test_assert_check_area_mat_mat()` - 18 edges
9. `HAL_SPI` - 18 edges
10. `ESP32_SPI` - 18 edges

## Surprising Connections (you probably didn't know these)
- `EKF Process & UpdateRefMeasurement Recursion` --semantically_similar_to--> `EKF Roll & Pitch Task (100Hz)`  [INFERRED] [semantically similar]
  managed_components/espressif__esp-dsp/modules/kalman/ekf_imu13states/docs/README.md → documents/RTOS Architecture.pdf
- `3. Impact & Architectural Constraints` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0013-sdCardSpiStorageAndFlightLoggingArchitecture.md → src/core/blackbox/sd_logger_task.cpp
- `3. Immediate Architectural Decision: Feature Deactivation` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0014-sdCardLoggingTemporaryDeprecationAndFindings.md → src/core/blackbox/sd_logger_task.cpp
- `4. Restoration & Hardening Roadmap (For Future Implementation)` --references--> `sd_logger_task()`  [INFERRED]
  documents/notes/0014-sdCardLoggingTemporaryDeprecationAndFindings.md → src/core/blackbox/sd_logger_task.cpp
- `Rule 5: Pure 4-Digit Incremental File Scheme & Self-Describing Binary Header` --references--> `LogFileHeader`  [INFERRED]
  documents/notes/0013-sdCardSpiStorageAndFlightLoggingArchitecture.md → src/drivers/storage/sd_card_driver.h

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

## Communities (182 total, 103 thin omitted)

### Community 1 - "dsps_cplx_gen_init.c"
Cohesion: 0.24
Nodes (8): cplx_gen_free(), dsps_cplx_gen_freq_get(), dsps_cplx_gen_freq_set(), dsps_cplx_gen_init(), dsps_cplx_gen_phase_get(), dsps_cplx_gen_phase_set(), dsps_cplx_gen_set(), error_msg_handler()

### Community 2 - "Mat"
Cohesion: 0.05
Nodes (49): ekf::dFdq(), ekf::dFdq_inv(), ekf::eul2rotm(), ekf::qProduct(), ekf::quat2eul(), ekf::quat2rotm(), ekf::rotm2eul(), ekf::rotm2quat() (+41 more)

### Community 3 - "dsps_fird_s16_aexx_free"
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
Cohesion: 0.18
Nodes (12): ESP32_I2C_ASYNC, ESP32_I2C_ASYNC::ESP32_I2C_ASYNC(), _frequency_hz, init, _initialized, _port, read_reg_async, read_reg_multi_async (+4 more)

### Community 9 - "test_mat_sub_f32.cpp"
Cohesion: 0.20
Nodes (26): Copy, Get, getROI, inverse, pinv, test_assert_check_area_mat_mat(), test_assert_equal_mat_mat(), "Mat class matrix subset" (+18 more)

### Community 10 - "mat.cpp"
Cohesion: 0.08
Nodes (17): augment, expHelper, eye, swapRows, Mat::augment(), Mat::Copy(), Mat::CopyHead(), Mat::det() (+9 more)

### Community 11 - "ekf"
Cohesion: 0.10
Nodes (21): ekf::RungeKutta(), ekf, CovariancePrediction, dFdq, dFdq_inv, eul2rotm, HP, Init (+13 more)

### Community 12 - "HMC5883L"
Cohesion: 0.09
Nodes (19): 1. Prime Directive: WS2812 is an External Hardware Peripheral, 2. Mandatory Mutex Encapsulation Rule, Zettel 0012: WS2812 External Hardware Treatment & Mutex Encapsulation Rule, HMC5883L, _dev_addr, get_all_data, get_all_data_raw, HMC5883L::HMC5883L() (+11 more)

### Community 13 - "SDCardDriver"
Cohesion: 0.14
Nodes (19): append_packet(), sd_logger_task(), SDCardDriver, _bytes_written, _cd_pin, close, _cs_pin, _file (+11 more)

### Community 15 - "Attitude Outer Loop Task (100Hz)"
Cohesion: 0.12
Nodes (19): Accel Gyro Calib & Filter Task (250Hz), Accel Gyro DAQ Task (500Hz), Altitude & Vz Estimator Task (100Hz), Attitude Inner Loop Task (250Hz, 4 Pri), Attitude Outer Loop Task (100Hz), Barometer Calib & Filter Task, Barometer DAQ Task, EKF Roll & Pitch Task (100Hz) (+11 more)

### Community 16 - "kalman_filter_demo.cpp"
Cohesion: 0.15
Nodes (14): app_fbm320_init(), app_mag3110_init(), app_main(), app_ssd1306_init(), display_3d_image(), draw_3d_image(), get_initial_pressure(), get_pressure_task() (+6 more)

### Community 18 - "dsps_fft2r_sc16_ansi.c"
Cohesion: 0.23
Nodes (12): microphone_read_task(), dsps_bit_rev_sc16_ansi(), dsps_cplx2real_sc16_ansi(), dsps_cplx2reC_sc16(), dsps_fft2r_init_sc16(), dsps_fft2r_sc16_ansi_(), dsps_gen_w_r2_sc16(), xtfixed_bf_1() (+4 more)

### Community 19 - "dsps_math_main.c"
Cohesion: 0.31
Nodes (4): app_main(), process_and_show(), app_main(), ShowIIRfilter()

### Community 20 - "dsps_fft4r_fc32_ansi.c"
Cohesion: 0.36
Nodes (8): app_main(), dsp_power_of_two(), dsps_bit_rev4r_direct_fc32_ansi(), dsps_bit_rev4r_fc32(), dsps_cplx2real_fc32_ansi_(), dsps_fft4r_fc32_ansi_(), dsps_fft4r_init_fc32(), dsps_gen_bitrev4r_table()

### Community 21 - "matrix.h"
Cohesion: 0.26
Nodes (5): inverse3x3(), Matrix, norm(), normalize(), operator+()

### Community 22 - "WS2812"
Cohesion: 0.07
Nodes (29): WS2812, brightness, current_b, current_g, current_r, encode_and_transmit, init, is_on (+21 more)

### Community 23 - "Vec3"
Cohesion: 0.09
Nodes (23): PktMagRaw, mag, timestamp_us, type, Vec3, BleAccelPacket, accel, timestamp (+15 more)

### Community 24 - "init_display.c"
Cohesion: 0.21
Nodes (9): bsp_display_lcd_init(), bsp_enable_feature(), bsp_i2c_init(), bsp_spi_init(), display_init(), dsp_display_brightness_init(), dsp_display_new(), dsp_display_start_with_config() (+1 more)

### Community 25 - "How to use example"
Cohesion: 0.29
Nodes (6): Basic Math Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 26 - "config.h"
Cohesion: 0.17
Nodes (3): OtaDataChunk, data, length

### Community 27 - "ekf_imu13states.cpp"
Cohesion: 0.16
Nodes (14): app_main(), ekf::CovariancePrediction(), ekf_imu13states::Init(), ekf_imu13states::LinearizeFG(), ekf_imu13states::StateXdot(), ekf_imu13states::Test(), ekf_imu13states::TestFull(), ekf_imu13states::UpdateRefMeasurement() (+6 more)

### Community 28 - "Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout"
Cohesion: 0.10
Nodes (33): FC_v01 Firmware Architecture Specification, Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout, Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision, Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision, Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule, Zettel 0004: Multi-Type SIMD Acceleration & ESP-DSP Build Integration, Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing, Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions (+25 more)

### Community 29 - "Rect"
Cohesion: 0.14
Nodes (15): app_main(), Rect, areaRect, height, resizeRect, width, x, y (+7 more)

### Community 30 - "audio_amp_main.c"
Cohesion: 0.20
Nodes (9): audio_process_task(), audio_read_task(), buttons_process_task(), convert_float2short(), convert_short2float(), digitalLimiter(), dsps_biquad_gen_highShelf_f32(), dsps_biquad_gen_lowShelf_f32() (+1 more)

### Community 31 - "dsps_biquad_gen_f32.c"
Cohesion: 0.36
Nodes (8): dsps_biquad_gen_allpass180_f32(), dsps_biquad_gen_allpass360_f32(), dsps_biquad_gen_bpf0db_f32(), dsps_biquad_gen_bpf_f32(), dsps_biquad_gen_hpf_f32(), dsps_biquad_gen_lpf_f32(), dsps_biquad_gen_notch_f32(), dsps_biquad_gen_peakingEQ_f32()

### Community 32 - "ble_driver.cpp"
Cohesion: 0.19
Nodes (15): Strict 1:1 Task-to-Characteristic Isolation, OtaControlCallbacks, OtaDataCallbacks, send_ble_accel_filt_notification(), send_ble_accel_raw_notification(), send_ble_att_euler_notification(), send_ble_att_quat_notification(), send_ble_baro_filt_notification() (+7 more)

### Community 33 - "math"
Cohesion: 0.12
Nodes (8): app_main(), process_and_show(), dsps_wind_blackman_f32(), dsps_wind_blackman_harris_f32(), dsps_wind_blackman_nuttall_f32(), dsps_wind_flat_top_f32(), dsps_wind_hann_f32(), dsps_wind_nuttall_f32()

### Community 34 - "3d_kalman_demo.cpp"
Cohesion: 0.35
Nodes (9): app_init(), app_main(), init_3d_matrix_struct(), read_bmi270_data(), read_bmi270_reg(), read_bmm150_data(), write_bmi270_data(), write_bmi270_reg() (+1 more)

### Community 35 - "CPU Logo Converted 3D Matrix Image"
Cohesion: 0.33
Nodes (6): CPU Logo Converted 3D Matrix Image, Monochrome CPU Microcontroller Dot-Matrix Representation, CPU Logo Inverted Color Monochromatic Preview (64x64), Inverted Monochromatic Dot-Matrix Display Buffer, 2D Chip Graphic Input for 3D Matrix Transformation, CPU Microcontroller 2D Logo Source Image (PNG)

### Community 36 - "azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.21
Nodes (6): app_main(), app_ssd1306_init(), display_3d_image(), draw_3d_image_task(), init_3d_matrix_struct(), init_perspective_matrix()

### Community 37 - "m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.22
Nodes (3): app_main(), display_3d_image(), init_3d_matrix_struct()

### Community 38 - "How to use example"
Cohesion: 0.29
Nodes (6): 2D convolution Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 40 - "FcBleServerCallbacks"
Cohesion: 0.24
Nodes (7): is_ble_ota_active(), set_ble_connected(), FcBleServerCallbacks, onConnect, onDisconnect, p_led, WS2812

### Community 41 - "sysview_tracing.h"
Cohesion: 0.33
Nodes (4): app_main(), init_ipc(), sysview_init_markers(), init

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

### Community 46 - "lpf.h"
Cohesion: 0.32
Nodes (6): LowPassFilter1stOrder, initialized, x_prev, y_prev, lpf_apply(), lpf_init()

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

### Community 52 - "freertos"
Cohesion: 0.31
Nodes (4): convert_to_rgb(), image_display_task(), spectrum2d_picture(), spectrum2d_picture_init()

### Community 53 - "dsps_fft2r_fc32_ansi.c"
Cohesion: 0.37
Nodes (11): dsp_is_power_of_two(), dsps_bit_rev2r_fc32(), dsps_bit_rev_fc32_ansi(), dsps_bit_rev_lookup_fc32_ansi(), dsps_cplx2reC_fc32_ansi(), dsps_fft2r_fc32_ansi_(), dsps_fft2r_init_fc32(), dsps_gen_bitrev2r_table() (+3 more)

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

### Community 75 - "stdio"
Cohesion: 0.13
Nodes (7): main(), test_fft2r(), dsps_biquad_f32_ansi(), main(), test_iir_biquad(), main(), test_mmult()

### Community 77 - "dsp_common.h"
Cohesion: 0.11
Nodes (4): tie_log(), dsps_dctiv_f32(), dsps_dstiv_f32(), dsps_firmr_s16_ansi()

### Community 82 - "ChannelTracker"
Cohesion: 0.50
Nodes (3): ChannelTracker, has_logged, last_ts

### Community 127 - "sd_logger_task.cpp"
Cohesion: 0.07
Nodes (25): PktAttEuler, euler, timestamp_us, type, PktAttQuat, q, timestamp_us, type (+17 more)

### Community 128 - "HAL_I2C"
Cohesion: 0.14
Nodes (14): HAL_I2C, read_reg, read_reg_multi, write_reg, write_reg_multi, ESP32_I2C, ESP32_I2C::ESP32_I2C(), init (+6 more)

### Community 139 - "ble_telem_tasks.cpp"
Cohesion: 0.27
Nodes (13): accel_gyro_lpf_task(), mag_lpf_task(), accel_gyro_daq_task(), mag_daq_task(), ble_accel_filt_trans_task(), ble_accel_raw_trans_task(), ble_gyro_filt_trans_task(), ble_gyro_raw_trans_task() (+5 more)

### Community 140 - "RingBuffer"
Cohesion: 0.23
Nodes (3): RingBuffer, _buffer, _head

### Community 141 - "stdint"
Cohesion: 0.13
Nodes (4): dsps_fft2r_rev_tables_init_fc32(), dsps_fft2r_deinit_fc32(), dsps_fft4r_rev_tables_init_fc32(), dsps_fft4r_deinit_fc32()

### Community 143 - "ekf_imu13states"
Cohesion: 0.12
Nodes (15): image_3d_matrix_kalman_s, ekf13, matrix_len, image_3d_matrix_s, matrix_len, ekf_imu13states, accel0, Init (+7 more)

### Community 151 - "HAL_SPI"
Cohesion: 0.18
Nodes (10): HAL_SPI, get_free_bytes, get_total_bytes, init, is_card_inserted, is_fatfs_mounted, mount_fatfs, transfer (+2 more)

### Community 153 - "2. Core Architectural Decisions"
Cohesion: 0.22
Nodes (11): 2. Core Architectural Decisions, Rule 1: 10-Second Boot-Up Security Window & Dynamic Memory Reclamation, Rule 2: Layer 4 Task Identity & Zettel 0006 Rule 3 Preservation, Rule 3: NimBLE Host Task Decoupling & Stack Protection, Rule 4: Client-Side ATT MTU Adaptation & Error Unblocking, Rule 5: Environment Simplification & Local Project Compilation, Rule 6: Bluetooth 5.0 2M PHY Negotiation & DLE 251 Link Layer Acceleration, Rule 7: Windowed WRITE_NR Pipelining with SPI Flash Backpressure Flow Control (+3 more)

### Community 154 - "LogFileHeader"
Cohesion: 0.18
Nodes (11): LogFileHeader, board_name, boot_timestamp_us, circular_head_sector, limit_policy, magic, max_file_size_bytes, registered_packets_mask (+3 more)

### Community 155 - "2. Core Architectural Decisions"
Cohesion: 0.22
Nodes (9): 1. Metadata & Context, 2. Core Architectural Decisions, 3. Impact & Architectural Constraints, Rule 1: Unified `HAL_SPI` Abstraction & Shared Bus Decoupling (Layer 1), Rule 2: Peripheral Driver Object Ownership (`SDCardDriver`, Layer 2), Rule 3: Autonomous 100 Hz Periodic Task & Zero-Overhead Ring Buffer Snapshot Draining (Layer 4), Rule 4: 512-Byte Sector Accumulator & Dual-Trigger Flush, Rule 5: Pure 4-Digit Incremental File Scheme & Self-Describing Binary Header (+1 more)

### Community 160 - "test_dsps_conv_f32_ansi.c"
Cohesion: 0.18
Nodes (3): dsps_ccorr_f32_ansi(), dsps_conv_f32_ansi(), dsps_conv_f32_ref()

### Community 161 - "HAL_I2C_ASYNC"
Cohesion: 0.22
Nodes (6): HAL_I2C_ASYNC, init, read_reg_async, read_reg_multi_async, write_reg_async, write_reg_multi_async

### Community 162 - "graphics_support.cpp"
Cohesion: 0.33
Nodes (8): dispaly_esp_text(), print_matrix(), update_rotation_matrix(), update_scaling_matrix(), update_translation_matrix(), draw_3d_image_task(), display_3d_image(), draw_3d_image_task()

### Community 163 - "test_mat_f32.cpp"
Cohesion: 0.19
Nodes (6): dspm_mult_f32_ansi(), "Mat class ", "Mat class basic operations", "Mat class check solve ", "Mat class operators", "mat.cpp functionality"

### Community 164 - "dsps_view"
Cohesion: 0.32
Nodes (5): app_main(), dsps_tone_gen_f32(), dsps_view(), dsps_view_s16(), dsps_view_spectrum()

### Community 165 - "dsps_fir_main.c"
Cohesion: 0.32
Nodes (4): app_main(), generate_FIR_coefficients(), show_FFT(), dsps_fird_init_f32()

### Community 166 - "dsps_dct_f32.c"
Cohesion: 0.53
Nodes (4): dsps_dct_f32(), dsps_dct_f32_ref(), dsps_dct_inv_f32(), dsps_dct_inverce_f32_ref()

### Community 169 - "PktImuFilt"
Cohesion: 0.40
Nodes (5): PktImuFilt, accel, gyro, timestamp_us, type

### Community 170 - "BleBaroPacket"
Cohesion: 0.50
Nodes (3): BleBaroPacket, pressure, timestamp

## Knowledge Gaps
- **321 isolated node(s):** `matrix_len`, `matrix_len`, `ekf13`, `Process`, `Init` (+316 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 627 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **103 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Mat` connect `Mat` to `graphics_support.cpp`, `test_mat_f32.cpp`, `azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp`, `m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp`, `esp_dsp.h`, `test_mat_sub_f32.cpp`, `mat.cpp`, `ekf`, `ekf_imu13states`, `kalman_filter_demo.cpp`, `ekf_imu13states.cpp`, `Rect`, `simd.cpp`?**
  _High betweenness centrality (0.067) - this node is a cross-community bridge._
- **Why does `HAL_SPI` connect `HAL_SPI` to `app.cpp`, `SDCardDriver`, `ESP32_SPI`, `config.h`, `2. Core Architectural Decisions`?**
  _High betweenness centrality (0.030) - this node is a cross-community bridge._
- **Why does `sd_logger_task()` connect `SDCardDriver` to `Zettel 0014: MicroSD Flight Logging Temporary Deprecation, FatFS Metadata Analysis & Future Architecture Roadmap`, `ble_telem_tasks.cpp`, `ChannelTracker`, `2. Core Architectural Decisions`, `sd_logger_task.cpp`?**
  _High betweenness centrality (0.030) - this node is a cross-community bridge._
- **What connects `matrix_len`, `matrix_len`, `ekf13` to the rest of the system?**
  _321 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `string` be split into smaller, more focused modules?**
  _Cohesion score 0.12794612794612795 - nodes in this community are weakly interconnected._
- **Should `Mat` be split into smaller, more focused modules?**
  _Cohesion score 0.05117845117845118 - nodes in this community are weakly interconnected._
- **Should `dsps_fird_s16_aexx_free` be split into smaller, more focused modules?**
  _Cohesion score 0.059743954480796585 - nodes in this community are weakly interconnected._