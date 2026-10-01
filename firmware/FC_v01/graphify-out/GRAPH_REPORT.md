# Graph Report - FC_v01  (2026-09-30)

## Corpus Check
- 389 files · ~220,981 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 127 file(s) not represented in the graph (top: .S 92, .defaults 15, (none) 11)

## Summary
- 1404 nodes · 3071 edges · 148 communities (62 shown, 86 thin omitted)
- Extraction: 90% EXTRACTED · 10% INFERRED · 0% AMBIGUOUS · INFERRED: 303 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4ce19fda`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- esp_log.h
- dsps_cplx_gen_init.c
- Mat
- dsps_fird_init_s16.c
- ESP-DSP Library Overview
- MPU6050
- Image3DMatrix
- ESP32_I2C_ASYNC
- test_mat_sub_f32.cpp
- dsps_addc.h
- ekf_imu13states
- HMC5883L
- dsp_err.h
- Attitude Outer Loop Task (100Hz)
- kalman_filter_demo.cpp
- ekf
- ekf.cpp
- dsps_view
- Vec3
- dsp_common.h
- app_main
- init_display.c
- How to use example
- math
- ekf_imu13states.cpp
- Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout
- Rect
- ShowIIRfilter
- HAL_I2C
- config.h
- dsps_biquad_gen_f32.c
- 3d_kalman_demo.cpp
- CPU Logo Converted 3D Matrix Image
- dsps_dct_f32.c
- RingBuffer
- How to use example
- stdint
- onConnect
- test_dsps_memcpy_memset.c
- How to use example
- m_test_data_s
- main/main.c
- azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp
- ble_driver.cpp
- 5-Layer Decoupled Architecture
- How to use example
- How to use example
- How to use example
- dsps_biquad_f32_ansi
- dsps_sqrt_f32_ansi.c
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
- dsps_fft4r.h
- Standardized RTOS Timing Analysis Rules
- ESP-DSP Component Registration
- Layer 1 Zero-Fallback Rule
- app_main
- examples/README.md
- dsps_corr_f32_ansi.c
- dsps_dotprod_s16_ansi.c
- dsps_dp_s8_ansi.c
- dsps_dotprod_f32_ansi.c
- dsps_dotprode_f32_ansi.c
- dsps_fird_s16_ansi.c
- dsps_fird_f32_ansi.c
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
- Tustin Bilinear 1st-Order Low-Pass Filter
- dsps_fft2r_sc16_ansi.c
- dsps_conv.h
- dsps_fft2r_fc32_ansi.c
- m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp
- audio_amp_main.c
- dsps_fft4r_fc32_ansi.c
- HAL_I2C_ASYNC
- LowPassFilter1stOrder
- C_sub_check

## God Nodes (most connected - your core abstractions)
1. `Mat` - 98 edges
2. `MPU6050` - 29 edges
3. `ekf` - 26 edges
4. `ekf_imu13states` - 21 edges
5. `HMC5883L` - 19 edges
6. `test_assert_check_area_mat_mat()` - 18 edges
7. `dsp_is_power_of_two()` - 17 edges
8. `getROI` - 17 edges
9. `Image3DMatrix` - 16 edges
10. `Get` - 15 edges

## Surprising Connections (you probably didn't know these)
- `EKF Process & UpdateRefMeasurement Recursion` --semantically_similar_to--> `EKF Roll & Pitch Task (100Hz)`  [INFERRED] [semantically similar]
  managed_components/espressif__esp-dsp/modules/kalman/ekf_imu13states/docs/README.md → documents/RTOS Architecture.pdf
- `FreeRTOS Task Scoping Architecture Rule` --semantically_similar_to--> `Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision`  [INFERRED] [semantically similar]
  .agents/AGENTS.md → documents/notes/0002-taskEnumsAndHalAbstractionDecision.md
- `Centralized Task Parameters Rule` --semantically_similar_to--> `Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing`  [INFERRED] [semantically similar]
  .agents/AGENTS.md → documents/notes/0006-taskCreationConfigEnumsAndTaskArgsPassingDecision.md
- `Strict 1:1 Task-to-Characteristic Isolation` --references--> `ble_vec3_raw_trans_task()`  [INFERRED]
  documents/notes/0009-distinctBleGattCharacteristicUuidIsolation.md → src/core/telemetry/telem_tasks.cpp
- `Dual-Trigger Flushing Mechanism` --references--> `ble_vec3_raw_trans_task()`  [INFERRED]
  documents/notes/0010-bleMinimumPacketFrequencyAndDualTriggerFlush.md → src/core/telemetry/telem_tasks.cpp

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

## Communities (148 total, 86 thin omitted)

### Community 0 - "esp_log.h"
Cohesion: 0.06
Nodes (25): dsps_conv_f32_ref(), dsps_firmr_f32_ansi(), "ekf_imu13states functionality gyro and magn", "ekf_imu13states functionality gyro only", "dspm_add_f32_ae32 benchmark", "dspm_add_f32_ae32 functionality", "dspm_add_f32_ansi benchmark", "dspm_add_f32_ansi functionality" (+17 more)

### Community 1 - "dsps_cplx_gen_init.c"
Cohesion: 0.16
Nodes (9): dsps_cplx_gen_ansi(), cplx_gen_free(), dsps_cplx_gen_freq_get(), dsps_cplx_gen_freq_set(), dsps_cplx_gen_init(), dsps_cplx_gen_phase_get(), dsps_cplx_gen_phase_set(), dsps_cplx_gen_set() (+1 more)

### Community 2 - "Mat"
Cohesion: 0.05
Nodes (48): Mat, abs_tol, adjoint, allocate, augment, bandSolve, block, clear (+40 more)

### Community 3 - "dsps_fird_init_s16.c"
Cohesion: 0.05
Nodes (18): dsps_16_array_rev(), dsps_fird_init_s16(), dsps_fird_s16_aexx_free(), dsps_firmr_init_s16(), dsps_fir_f32_ansi(), dsps_fir_f32_free(), dsps_fir_init_f32(), dsps_firmr_init_f32() (+10 more)

### Community 4 - "ESP-DSP Library Overview"
Cohesion: 0.05
Nodes (44): ESP-DSP GitLab Pre-Commit CI Job, ESP-DSP Pre-Commit Hook Configurations, ESP32-Azure IoT Kit 3D Graphics Demo Animation, 3D Matrix Transformation and Projection on LCD, esp-dsp-azure-board-app-3d-graphics Project, Azure 3D Graphics Main Component, Azure 3D Graphics IDF Component Manifest, Azure 3D Graphics Demo Application Guide (+36 more)

### Community 5 - "MPU6050"
Cohesion: 0.07
Nodes (23): MPU6050, _accel_raw_x, _accel_raw_y, _accel_raw_z, _accel_sens, _dev_addr, enable_bypass, enable_sleep (+15 more)

### Community 6 - "Image3DMatrix"
Cohesion: 0.08
Nodes (6): convert_img(), create_matrix(), fromat_comas_spaces(), Image3DMatrix, main(), ouptput_files_gen()

### Community 7 - "ESP32_I2C_ASYNC"
Cohesion: 0.09
Nodes (21): ESP32_I2C_ASYNC, ESP32_I2C_ASYNC::ESP32_I2C_ASYNC(), _frequency_hz, init, _initialized, _port, read_reg_async, read_reg_multi_async (+13 more)

### Community 9 - "test_mat_sub_f32.cpp"
Cohesion: 0.20
Nodes (26): Copy, Get, getROI, inverse, pinv, test_assert_check_area_mat_mat(), test_assert_equal_mat_mat(), "Mat class matrix subset" (+18 more)

### Community 11 - "ekf_imu13states"
Cohesion: 0.12
Nodes (15): image_3d_matrix_kalman_s, ekf13, matrix_len, image_3d_matrix_s, matrix_len, ekf_imu13states, accel0, Init (+7 more)

### Community 12 - "HMC5883L"
Cohesion: 0.11
Nodes (16): HMC5883L, _dev_addr, get_all_data, get_all_data_raw, HMC5883L::HMC5883L(), init, _inv_mag_sens, _mag_raw_x (+8 more)

### Community 13 - "dsp_err.h"
Cohesion: 0.09
Nodes (4): dspm_add_f32_ansi(), dspm_addc_f32_ansi(), dspm_mulc_f32_ansi(), dspm_sub_f32_ansi()

### Community 15 - "Attitude Outer Loop Task (100Hz)"
Cohesion: 0.12
Nodes (19): Accel Gyro Calib & Filter Task (250Hz), Accel Gyro DAQ Task (500Hz), Altitude & Vz Estimator Task (100Hz), Attitude Inner Loop Task (250Hz, 4 Pri), Attitude Outer Loop Task (100Hz), Barometer Calib & Filter Task, Barometer DAQ Task, EKF Roll & Pitch Task (100Hz) (+11 more)

### Community 16 - "kalman_filter_demo.cpp"
Cohesion: 0.16
Nodes (14): app_fbm320_init(), app_mag3110_init(), app_main(), app_ssd1306_init(), display_3d_image(), draw_3d_image(), get_initial_pressure(), get_pressure_task() (+6 more)

### Community 17 - "ekf"
Cohesion: 0.10
Nodes (21): ekf, CovariancePrediction, dFdq, dFdq_inv, eul2rotm, HP, Init, Km (+13 more)

### Community 18 - "ekf.cpp"
Cohesion: 0.10
Nodes (20): ekf::dFdq(), ekf::dFdq_inv(), ekf::eul2rotm(), ekf::qProduct(), ekf::quat2eul(), ekf::quat2rotm(), ekf::rotm2eul(), ekf::rotm2quat() (+12 more)

### Community 20 - "dsps_view"
Cohesion: 0.21
Nodes (8): app_main(), process_and_show(), app_main(), dsps_tone_gen_f32(), dsps_view(), dsps_view_s16(), dsps_view_spectrum(), dsps_wind_hann_f32()

### Community 21 - "Vec3"
Cohesion: 0.21
Nodes (7): inverse3x3(), Matrix, operator+(), Vec3, BleVec3Packet, data, timestamp

### Community 23 - "app_main"
Cohesion: 0.18
Nodes (5): app_main(), process_and_show(), dsps_wind_blackman_nuttall_f32(), dsps_wind_flat_top_f32(), dsps_wind_nuttall_f32()

### Community 24 - "init_display.c"
Cohesion: 0.21
Nodes (9): bsp_display_lcd_init(), bsp_enable_feature(), bsp_i2c_init(), bsp_spi_init(), display_init(), dsp_display_brightness_init(), dsp_display_new(), dsp_display_start_with_config() (+1 more)

### Community 25 - "How to use example"
Cohesion: 0.29
Nodes (6): Basic Math Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 26 - "math"
Cohesion: 0.20
Nodes (4): main(), test_fft2r(), main(), test_mmult()

### Community 27 - "ekf_imu13states.cpp"
Cohesion: 0.13
Nodes (18): app_main(), app_main(), ekf::CovariancePrediction(), ekf_imu13states::Init(), ekf_imu13states::LinearizeFG(), ekf_imu13states::StateXdot(), ekf_imu13states::Test(), ekf_imu13states::TestFull() (+10 more)

### Community 28 - "Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout"
Cohesion: 0.17
Nodes (18): FC_v01 Firmware Architecture Specification, Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout, Zettel 0001: Centralized Task Config, Middleware ISR Callbacks & RMT PPM Decoding Decision, Zettel 0002: System 4 Priority Levels & Target-Agnostic HAL Decision, Zettel 0003: Layer 1 SIMD HAL Architecture & Zero-Fallback Rule, Zettel 0004: Multi-Type SIMD Acceleration & ESP-DSP Build Integration, Zettel 0006: Centralized Task Parameter Enums & Direct Driver Passing, Zettel 0007: SystemView Instrumentation & RTOS Profiling Conventions (+10 more)

### Community 29 - "Rect"
Cohesion: 0.25
Nodes (8): Rect, areaRect, height, resizeRect, width, y, Mat::Get(), Mat::getROI()

### Community 30 - "ShowIIRfilter"
Cohesion: 0.40
Nodes (3): app_main(), ShowIIRfilter(), dsps_d_gen_f32()

### Community 31 - "HAL_I2C"
Cohesion: 0.14
Nodes (11): app_start(), ble_driver_init(), app_main(), init_ipc(), sysview_init_markers(), HAL_I2C, init, read_reg (+3 more)

### Community 33 - "dsps_biquad_gen_f32.c"
Cohesion: 0.26
Nodes (12): audio_process_task(), buttons_process_task(), dsps_biquad_gen_allpass180_f32(), dsps_biquad_gen_allpass360_f32(), dsps_biquad_gen_bpf0db_f32(), dsps_biquad_gen_bpf_f32(), dsps_biquad_gen_highShelf_f32(), dsps_biquad_gen_hpf_f32() (+4 more)

### Community 34 - "3d_kalman_demo.cpp"
Cohesion: 0.29
Nodes (9): app_init(), app_main(), init_3d_matrix_struct(), read_bmi270_data(), read_bmi270_reg(), read_bmm150_data(), write_bmi270_data(), write_bmi270_reg() (+1 more)

### Community 35 - "CPU Logo Converted 3D Matrix Image"
Cohesion: 0.33
Nodes (6): CPU Logo Converted 3D Matrix Image, Monochrome CPU Microcontroller Dot-Matrix Representation, CPU Logo Inverted Color Monochromatic Preview (64x64), Inverted Monochromatic Dot-Matrix Display Buffer, 2D Chip Graphic Input for 3D Matrix Transformation, CPU Microcontroller 2D Logo Source Image (PNG)

### Community 36 - "dsps_dct_f32.c"
Cohesion: 0.19
Nodes (6): dsps_dct_f32(), dsps_dct_f32_ref(), dsps_dct_inv_f32(), dsps_dct_inverce_f32_ref(), dsps_dctiv_f32(), dsps_dstiv_f32()

### Community 37 - "RingBuffer"
Cohesion: 0.24
Nodes (3): RingBuffer, _buffer, _head

### Community 38 - "How to use example"
Cohesion: 0.29
Nodes (6): 2D convolution Example, Build and flash, Configure the project, Example output, Hardware required, How to use example

### Community 39 - "stdint"
Cohesion: 0.13
Nodes (4): dsps_fft2r_rev_tables_init_fc32(), dsps_fft2r_deinit_fc32(), dsps_fft4r_rev_tables_init_fc32(), dsps_fft4r_deinit_fc32()

### Community 40 - "onConnect"
Cohesion: 0.38
Nodes (4): set_ble_connected(), FcBleServerCallbacks, onConnect, onDisconnect

### Community 43 - "How to use example"
Cohesion: 0.29
Nodes (6): Build and flash, Configure the project, Dot Product Calculation Example, Example output, Hardware required, How to use example

### Community 44 - "m_test_data_s"
Cohesion: 0.20
Nodes (10): m_test_data_s, A_start_col, A_start_row, B_start_col, B_start_row, C_start_col, C_start_row, k (+2 more)

### Community 45 - "main/main.c"
Cohesion: 0.31
Nodes (4): convert_to_rgb(), image_display_task(), spectrum2d_picture(), spectrum2d_picture_init()

### Community 46 - "azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.17
Nodes (14): app_main(), app_ssd1306_init(), dispaly_esp_text(), display_3d_image(), draw_3d_image_task(), init_3d_matrix_struct(), init_perspective_matrix(), print_matrix() (+6 more)

### Community 47 - "ble_driver.cpp"
Cohesion: 0.24
Nodes (15): Strict 1:1 Task-to-Characteristic Isolation, Dual-Trigger Flushing Mechanism, accel_gyro_daq_task(), mag_daq_task(), accel_gyro_lpf_task(), mag_lpf_task(), lpf_apply(), lpf_init() (+7 more)

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

### Community 52 - "dsps_biquad_f32_ansi"
Cohesion: 0.33
Nodes (3): dsps_biquad_f32_ansi(), main(), test_iir_biquad()

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

### Community 75 - "app_main"
Cohesion: 0.15
Nodes (6): app_main(), generate_FIR_coefficients(), show_FFT(), dsps_fird_init_f32(), dsps_wind_blackman_f32(), dsps_wind_blackman_harris_f32()

### Community 139 - "dsps_fft2r_sc16_ansi.c"
Cohesion: 0.18
Nodes (13): microphone_read_task(), dsps_bit_rev_sc16_ansi(), dsps_cplx2real_sc16_ansi(), dsps_cplx2reC_sc16(), dsps_fft2r_init_sc16(), dsps_fft2r_sc16_ansi_(), dsps_gen_w_r2_sc16(), xtfixed_bf_1() (+5 more)

### Community 140 - "dsps_conv.h"
Cohesion: 0.14
Nodes (3): dspi_conv_f32_ansi(), dsps_ccorr_f32_ansi(), dsps_conv_f32_ansi()

### Community 141 - "dsps_fft2r_fc32_ansi.c"
Cohesion: 0.37
Nodes (11): dsp_is_power_of_two(), dsps_bit_rev2r_fc32(), dsps_bit_rev_fc32_ansi(), dsps_bit_rev_lookup_fc32_ansi(), dsps_cplx2reC_fc32_ansi(), dsps_fft2r_fc32_ansi_(), dsps_fft2r_init_fc32(), dsps_gen_bitrev2r_table() (+3 more)

### Community 142 - "m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp"
Cohesion: 0.22
Nodes (3): app_main(), display_3d_image(), init_3d_matrix_struct()

### Community 143 - "audio_amp_main.c"
Cohesion: 0.27
Nodes (5): audio_read_task(), convert_float2short(), convert_short2float(), digitalLimiter(), dsps_mulc_f32_ansi()

### Community 144 - "dsps_fft4r_fc32_ansi.c"
Cohesion: 0.42
Nodes (8): app_main(), dsp_power_of_two(), dsps_bit_rev4r_direct_fc32_ansi(), dsps_bit_rev4r_fc32(), dsps_cplx2real_fc32_ansi_(), dsps_fft4r_fc32_ansi_(), dsps_fft4r_init_fc32(), dsps_gen_bitrev4r_table()

### Community 145 - "HAL_I2C_ASYNC"
Cohesion: 0.29
Nodes (6): HAL_I2C_ASYNC, init, read_reg_async, read_reg_multi_async, write_reg_async, write_reg_multi_async

### Community 146 - "LowPassFilter1stOrder"
Cohesion: 0.40
Nodes (5): LowPassFilter1stOrder, a, initialized, x_prev, y_prev

## Knowledge Gaps
- **213 isolated node(s):** `matrix_len`, `matrix_len`, `ekf13`, `Process`, `Init` (+208 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 481 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **86 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Mat` connect `Mat` to `esp_log.h`, `config.h`, `test_mat_sub_f32.cpp`, `ekf_imu13states`, `m5stack_core_s3/apps/3d_graphics/main/3d_graphics_demo.cpp`, `azure_board_apps/apps/3d_graphics/main/3d_graphics_demo.cpp`, `kalman_filter_demo.cpp`, `ekf`, `ekf.cpp`, `ekf_imu13states.cpp`, `Rect`?**
  _High betweenness centrality (0.102) - this node is a cross-community bridge._
- **Why does `Strict 1:1 Task-to-Characteristic Isolation` connect `ble_driver.cpp` to `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout`?**
  _High betweenness centrality (0.035) - this node is a cross-community bridge._
- **Why does `2. Core Architectural Decision` connect `Zettel 0000: Firmware 5-Layer Architectural Decision & Directory Layout` to `ble_driver.cpp`?**
  _High betweenness centrality (0.035) - this node is a cross-community bridge._
- **What connects `matrix_len`, `matrix_len`, `ekf13` to the rest of the system?**
  _213 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `esp_log.h` be split into smaller, more focused modules?**
  _Cohesion score 0.06082597635792309 - nodes in this community are weakly interconnected._
- **Should `Mat` be split into smaller, more focused modules?**
  _Cohesion score 0.04644808743169399 - nodes in this community are weakly interconnected._
- **Should `dsps_fird_init_s16.c` be split into smaller, more focused modules?**
  _Cohesion score 0.052525252525252523 - nodes in this community are weakly interconnected._