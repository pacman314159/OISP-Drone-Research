#include <Arduino.h>
#include "core/sensor/calib_tasks.h"
#include "core/sensor/calib.h"
#include "core/math/ellipsoid_fit.h"
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/sysview_tracing.h"
#include "platforms/hal/hal_simd.h"

// =============================================================================
// SINGULAR CALIBRATION TASKS
// =============================================================================

// Gyroscope Calibration: Stationary zero-bias multi-sample averaging
void task_calib_gyro(void* arg){
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_CALIB_GYRO_FREQ_HZ);

  Serial.printf("[CALIB_GYRO] Task started. Entering %u ms settling delay...\n",
                static_cast<unsigned int>(CALIB_SETTLING_DELAY_MS));

  // Report settling phase to FSM coordinator
  CalibReportMsg settle_msg = {
    .target       = CALIB_TARGET_GYRO,
    .status       = CALIB_STATUS_SETTLING,
    .current_step = 0,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &settle_msg, 0);

  vTaskDelay(pdMS_TO_TICKS(CALIB_SETTLING_DELAY_MS));

  // Report active sampling transition to FSM coordinator
  CalibReportMsg sample_msg = {
    .target       = CALIB_TARGET_GYRO,
    .status       = CALIB_STATUS_SAMPLING,
    .current_step = 0,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &sample_msg, 0);

  Serial.println("[CALIB_GYRO] Settling complete. Collecting stationary samples...");

  uint32_t sample_count = 0;
  const uint32_t target_samples = 400; // 2 seconds at 200 Hz
  Vec3<float> sum_gyro{0.0f, 0.0f, 0.0f};
  bool completed = false;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(!completed){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(TASK_CALIB_GYRO_ID);

    if(!gyro_raw.empty()){
      sum_gyro += gyro_raw.front();
      sample_count++;
    }

    // Periodic healthcheck report to FSM at 10 Hz (every 20 samples at 200 Hz)
    if(sample_count % 20 == 0){
      CalibReportMsg msg = {
        .target       = CALIB_TARGET_GYRO,
        .status       = CALIB_STATUS_HEALTHCHECK,
        .current_step = 0,
        .progress_pct = static_cast<uint8_t>((sample_count * 100) / target_samples),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &msg, 0);
    }

    if(sample_count >= target_samples) completed = true;

    SYSVIEW_END(TASK_CALIB_GYRO_ID);
  }

  // Calculate average zero-rate bias offset and commit to Flash NVS
  Vec3<float> avg_bias = sum_gyro * (1.0f / static_cast<float>(target_samples));
  calib_get_data().gyro_bias = avg_bias;
  bool saved = calib_save_to_nvs();
  Serial.printf("[CALIB_GYRO] Bias: [%.4f, %.4f, %.4f] deg/s. Saved to NVS: %s\n",
                avg_bias.x, avg_bias.y, avg_bias.z, saved ? "SUCCESS" : "FAILED");

  // Report successful completion to FSM coordinator
  CalibReportMsg done_msg = {
    .target       = CALIB_TARGET_GYRO,
    .status       = CALIB_STATUS_COMPLETED,
    .current_step = 1,
    .progress_pct = 100,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &done_msg, portMAX_DELAY);

  Serial.println("[CALIB_GYRO] Completed. Halting and awaiting FSM deletion...");
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

// Barometer Calibration: Ground reference pressure P0 acquisition
void task_calib_baro(void* arg){
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_CALIB_BARO_FREQ_HZ);
  TickType_t last_wake_time = xTaskGetTickCount();

  uint32_t sample_count = 0;
  const uint32_t target_samples = 150; // 3 seconds at 50 Hz
  bool completed = false;

  Serial.println("[CALIB_BARO] Task started. Collecting ground pressure baseline...");

  while(!completed){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(TASK_CALIB_BARO_ID);

    // [DAQ Placeholder]: Retrieve baro pressure samples from ring buffer and accumulate
    sample_count++;

    // Periodic healthcheck report to FSM at 5 Hz (every 10 samples at 50 Hz)
    if(sample_count % 10 == 0){
      CalibReportMsg msg = {
        .target       = CALIB_TARGET_BARO,
        .status       = CALIB_STATUS_HEALTHCHECK,
        .current_step = 0,
        .progress_pct = static_cast<uint8_t>((sample_count * 100) / target_samples),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &msg, 0);
    }

    if(sample_count >= target_samples) completed = true;

    SYSVIEW_END(TASK_CALIB_BARO_ID);
  }

  // [Persistence Placeholder]: Calculate ground reference P0 and save to Flash NVS

  // Report successful completion to FSM coordinator
  CalibReportMsg done_msg = {
    .target       = CALIB_TARGET_BARO,
    .status       = CALIB_STATUS_COMPLETED,
    .current_step = 1,
    .progress_pct = 100,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &done_msg, portMAX_DELAY);

  Serial.println("[CALIB_BARO] Completed. Halting and awaiting FSM deletion...");
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

// =============================================================================
// ITERATIVE CALIBRATION TASKS
// =============================================================================

// Accelerometer Calibration: 6-Point interactive tumble + Ellipsoid fit solver
void task_calib_accel(void* arg){
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_CALIB_ACCEL_FREQ_HZ);
  TickType_t last_wake_time = xTaskGetTickCount();

  uint8_t current_orientation = 0;
  const uint8_t total_orientations = 6;
  uint32_t loop_counter = 0;
  bool daq_completed = false;

  Serial.println("[CALIB_ACCEL] Task started. Entering 6-point interactive acquisition...");

  // Phase 1: Interactive Data Acquisition across 6 orientations
  while(!daq_completed){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(TASK_CALIB_ACCEL_ID);

    loop_counter++;

    // [Stationary Check Placeholder]:
    // Wait for user to place drone on side 'current_orientation' and hold still.
    // When variance stays below threshold for 2.0s, lock side:
    // (Simulated placeholder condition every 3 seconds for framework verification)
    if(loop_counter % 600 == 0){
      current_orientation++;

      CalibReportMsg lock_msg = {
        .target       = CALIB_TARGET_ACCEL,
        .status       = CALIB_STATUS_STEP_LOCKED,
        .current_step = current_orientation,
        .progress_pct = static_cast<uint8_t>((current_orientation * 100) / total_orientations),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &lock_msg, 0);

      Serial.printf("[CALIB_ACCEL] Orientation %d of 6 locked!\n", current_orientation);

      if(current_orientation >= total_orientations) daq_completed = true;
    }else if(loop_counter % 20 == 0){
      // Periodic healthcheck report at 10 Hz
      CalibReportMsg msg = {
        .target       = CALIB_TARGET_ACCEL,
        .status       = CALIB_STATUS_HEALTHCHECK,
        .current_step = current_orientation,
        .progress_pct = static_cast<uint8_t>((current_orientation * 100) / total_orientations),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &msg, 0);
    }

    SYSVIEW_END(TASK_CALIB_ACCEL_ID);
  }

  // Phase 2: Iterative Ellipsoid Fit Numerical Optimization
  // Must use vTaskDelayUntil across iterations to prevent 100% CPU lockup
  Serial.println("[CALIB_ACCEL] Running iterative ellipsoid fit algorithm...");
  const uint32_t max_solver_iterations = 50;
  for(uint32_t iter = 0; iter < max_solver_iterations; ++iter){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(TASK_CALIB_ACCEL_ID);

    // [Solver Step Placeholder]: Gradient descent step on ellipsoid error metric

    if(iter % 10 == 0){
      CalibReportMsg solver_msg = {
        .target       = CALIB_TARGET_ACCEL,
        .status       = CALIB_STATUS_COMPUTING,
        .current_step = 6,
        .progress_pct = static_cast<uint8_t>((iter * 100) / max_solver_iterations),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &solver_msg, 0);
    }

    SYSVIEW_END(TASK_CALIB_ACCEL_ID);
  }

  // Commit accelerometer calibration parameters to Flash NVS
  bool saved_accel = calib_save_to_nvs();
  Serial.printf("[CALIB_ACCEL] Completed. Saved to NVS: %s\n", saved_accel ? "SUCCESS" : "FAILED");

  // Report final completion to FSM coordinator
  CalibReportMsg done_msg = {
    .target       = CALIB_TARGET_ACCEL,
    .status       = CALIB_STATUS_COMPLETED,
    .current_step = 6,
    .progress_pct = 100,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &done_msg, portMAX_DELAY);

  Serial.println("[CALIB_ACCEL] Completed. Halting and awaiting FSM deletion...");
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

// Static point cloud buffer in BSS (zero heap allocation, fits within SRAM)
static Vec3<float> s_mag_cloud[CALIB_MAG_SAMPLE_COUNT];

// Magnetometer Calibration: Continuous tumble rotation + SIMD Ellipsoid fit solver
void task_calib_mag(void* arg){
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_CALIB_MAG_FREQ_HZ);
  TickType_t last_wake_time = xTaskGetTickCount();

  uint16_t cloud_head = 0;
  uint16_t cloud_count = 0;
  uint32_t loop_counter = 0;
  Vec3<float> last_sample{0.0f, 0.0f, 0.0f};
  bool has_first_sample = false;
  bool daq_completed = false;

  Serial.println("[CALIB_MAG] Task started. Rotate drone indefinitely across all 3D axes.");
  Serial.println("[CALIB_MAG] Press USER BUTTON to stop acquisition and begin optimization.");

  // Phase 1: Real-time user continuous tumble rotation point-cloud acquisition
  while(!daq_completed){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(TASK_CALIB_MAG_ID);

    loop_counter++;

    // Ingest latest un-filtered raw measurement from ipc.h
    if(!mag_raw.empty()){
      Vec3<float> raw = mag_raw.front();
      if(!has_first_sample){
        s_mag_cloud[cloud_head] = raw;
        cloud_head = (cloud_head + 1) % CALIB_MAG_SAMPLE_COUNT;
        cloud_count = 1;
        last_sample = raw;
        has_first_sample = true;
      }else{
        Vec3<float> diff;
        hal_simd::sub(raw.m, last_sample.m, diff.m, 3);
        float dist_sq = hal_simd::dot_product(diff.m, diff.m, 3);
        if(dist_sq >= (CALIB_MAG_MIN_DIST_GAUSS * CALIB_MAG_MIN_DIST_GAUSS)){
          s_mag_cloud[cloud_head] = raw;
          cloud_head = (cloud_head + 1) % CALIB_MAG_SAMPLE_COUNT;
          if(cloud_count < CALIB_MAG_SAMPLE_COUNT) cloud_count++;
          last_sample = raw;
        }
      }
    }

    // Periodic healthcheck & progress report at 10 Hz (every 10 cycles at 100 Hz)
    if(loop_counter % 10 == 0){
      CalibReportMsg msg = {
        .target       = CALIB_TARGET_MAG,
        .status       = CALIB_STATUS_SAMPLING,
        .current_step = 0,
        .progress_pct = static_cast<uint8_t>((cloud_count * 100) / CALIB_MAG_SAMPLE_COUNT),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &msg, 0);
    }

    // Check for user completion trigger via hardware button notification from FSM
    if(ulTaskNotifyTake(pdTRUE, 0) > 0){
      if(cloud_count >= CALIB_MAG_MIN_SAMPLES){
        daq_completed = true;
        Serial.printf("[CALIB_MAG] Acquisition stopped by user! Collected %u diverse points.\n", cloud_count);
      }else{
        Serial.printf("[CALIB_MAG] Warning: Insufficient points (%u < %u). Keep tumbling drone!\n",
                      cloud_count, CALIB_MAG_MIN_SAMPLES);
      }
    }

    SYSVIEW_END(TASK_CALIB_MAG_ID);
  }

  // Phase 2: Iterative 3D Rotated Ellipsoid Fit Numerical Optimization (SIMD Adam)
  Serial.printf("[CALIB_MAG] Starting SIMD Adam optimization on %u points...\n", cloud_count);

  CalibReportMsg comp_msg = {
    .target       = CALIB_TARGET_MAG,
    .status       = CALIB_STATUS_COMPUTING,
    .current_step = 1,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &comp_msg, 0);

  static EllipsoidFitState fit_state;
  ellipsoid_fit_init(fit_state, s_mag_cloud, cloud_count,
                     CALIB_MAG_MAX_EPOCHS, CALIB_MAG_ADAM_LR, CALIB_MAG_ADAM_TOL);

  Serial.printf("[CALIB_MAG] Initial RMSE: %.6f. Running %u epochs per cycle...\n",
                fit_state.initial_rmse, CALIB_MAG_EPOCHS_PER_CYCLE);

  bool opt_done = false;
  while(!opt_done){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(TASK_CALIB_MAG_ID);

    // Compute time-sliced Adam optimization epochs
    opt_done = ellipsoid_fit_step(fit_state, s_mag_cloud, cloud_count, CALIB_MAG_EPOCHS_PER_CYCLE);

    // Periodic progress report at 10 Hz
    if(fit_state.current_epoch % 10 == 0 || opt_done){
      CalibReportMsg solver_msg = {
        .target       = CALIB_TARGET_MAG,
        .status       = CALIB_STATUS_COMPUTING,
        .current_step = 1,
        .progress_pct = static_cast<uint8_t>((fit_state.current_epoch * 100) / fit_state.max_epochs),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &solver_msg, 0);
    }

    SYSVIEW_END(TASK_CALIB_MAG_ID);
  }

  // Phase 3: Extract parameters, package unit sphere matrix, and save to Flash NVS
  EllipsoidParameters params;
  ellipsoid_fit_get_params(fit_state, params);

  Serial.printf("[CALIB_MAG] Optimization finished in %u epochs (converged=%s)!\n",
                params.epochs_run, params.converged ? "YES" : "NO");
  Serial.printf("[CALIB_MAG] Initial RMSE: %.6f -> Final RMSE: %.6f\n",
                params.initial_rmse, params.final_rmse);
  Serial.printf("[CALIB_MAG] Calibrated Model Parameters (9-DOF):\n");
  Serial.printf("  Hard-Iron Center v0 (Gauss): [%.5f, %.5f, %.5f]\n",
                params.v0.x, params.v0.y, params.v0.z);
  Serial.printf("  Soft-Iron Matrix W (Unit Sphere):\n");
  Serial.printf("    [%.5f, %.5f, %.5f]\n", params.W(0,0), params.W(0,1), params.W(0,2));
  Serial.printf("    [%.5f, %.5f, %.5f]\n", params.W(1,0), params.W(1,1), params.W(1,2));
  Serial.printf("    [%.5f, %.5f, %.5f]\n", params.W(2,0), params.W(2,1), params.W(2,2));
  Serial.printf("  Geomagnetic Magnitude B0: %.5f Gauss\n", params.B0);

  // Store calibration parameters: W maps directly to unit sphere for Extended Kalman Filter
  calib_data.mag_bias      = params.v0;
  calib_data.mag_soft_iron = params.W;
  calib_data.mag_field_ref = params.B0;

  bool saved_mag = calib_save_to_nvs();
  Serial.printf("[CALIB_MAG] Completed. Saved to NVS: %s\n", saved_mag ? "SUCCESS" : "FAILED");

  // Report final completion to FSM coordinator
  CalibReportMsg done_msg = {
    .target       = CALIB_TARGET_MAG,
    .status       = CALIB_STATUS_COMPLETED,
    .current_step = 1,
    .progress_pct = 100,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &done_msg, portMAX_DELAY);

  Serial.println("[CALIB_MAG] Completed. Halting and awaiting FSM deletion...");
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}
