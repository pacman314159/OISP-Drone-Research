#include <Arduino.h>
#include "core/sensor/calib.h"
#include "core/sensor/calib_tasks.h"
#include "core/math/ellipsoid_fit.h"
#include "config.h"
#include "middleware/ipc.h"

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
  const uint32_t target_samples = CALIB_GYRO_SAMPLE_COUNT;
  Vec3<float> sum_gyro{0.0f, 0.0f, 0.0f};
  bool completed = false;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(!completed){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

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

  Serial.printf("[CALIB_BARO] Task started. Entering %u ms settling delay...\n",
                static_cast<unsigned int>(CALIB_SETTLING_DELAY_MS));

  // Report settling phase to FSM coordinator
  CalibReportMsg settle_msg = {
    .target       = CALIB_TARGET_BARO,
    .status       = CALIB_STATUS_SETTLING,
    .current_step = 0,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &settle_msg, 0);

  vTaskDelay(pdMS_TO_TICKS(CALIB_SETTLING_DELAY_MS));

  // Report active sampling transition to FSM coordinator
  CalibReportMsg sample_msg = {
    .target       = CALIB_TARGET_BARO,
    .status       = CALIB_STATUS_SAMPLING,
    .current_step = 0,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &sample_msg, 0);

  Serial.println("[CALIB_BARO] Settling complete. Collecting ground pressure baseline...");

  uint32_t sample_count = 0;
  const uint32_t target_samples = CALIB_BARO_SAMPLE_COUNT;
  double sum_pressure = 0.0;
  uint32_t last_baro_ts = 0;
  bool completed = false;

  TickType_t last_wake_time = xTaskGetTickCount();
  while(!completed){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(!pressure.empty()){
      uint32_t current_ts = pressure_timestamp.front();
      if(current_ts != last_baro_ts){
        last_baro_ts = current_ts;
        float p = pressure.front();
        if(p >= SENSOR_STREAM_BARO_MIN_PA && p <= SENSOR_STREAM_BARO_MAX_PA){
          sum_pressure += static_cast<double>(p);
          sample_count++;
        }
      }
    }

    // Periodic healthcheck report to FSM at 5 Hz (every 10 samples at 50 Hz)
    if(sample_count > 0 && sample_count % 10 == 0){
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
  }

  // Calculate ground reference P0 and commit to Flash NVS
  float avg_p0 = (sample_count > 0) ? static_cast<float>(sum_pressure / static_cast<double>(sample_count)) : BARO_SEA_LEVEL_PRESSURE_PA;
  calib_get_data().baro_ref_pa = avg_p0;
  bool saved = calib_save_to_nvs();
  Serial.printf("[CALIB_BARO] Ground Reference P0: %.2f Pa. Saved to NVS: %s\n",
                avg_p0, saved ? "SUCCESS" : "FAILED");

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
  // Point cloud buffer on task stack (reclaimed when FSM deletes task upon completion)
  Vec3<float> accel_cloud[CALIB_ACCEL_SAMPLE_COUNT];
  Vec3<float> locked_orientations[CALIB_ACCEL_ORIENTATIONS_COUNT];

  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_CALIB_ACCEL_FREQ_HZ);

  Serial.printf("[CALIB_ACCEL] Task started. Entering %u ms settling delay...\n",
                static_cast<unsigned int>(CALIB_SETTLING_DELAY_MS));

  // Report settling phase to FSM coordinator (LED: Fast-blinking Amber)
  CalibReportMsg settle_msg = {
    .target       = CALIB_TARGET_ACCEL,
    .status       = CALIB_STATUS_SETTLING,
    .current_step = 0,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &settle_msg, 0);

  vTaskDelay(pdMS_TO_TICKS(CALIB_SETTLING_DELAY_MS));

  // Report active sampling transition to FSM coordinator (LED: Breathing Orange)
  CalibReportMsg sample_msg = {
    .target       = CALIB_TARGET_ACCEL,
    .status       = CALIB_STATUS_SAMPLING,
    .current_step = 0,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &sample_msg, 0);

  Serial.println("[CALIB_ACCEL] Settling complete. Place drone on each of 6 sides.");
  Serial.println("[CALIB_ACCEL] Hold drone stationary for 2.0s to lock each orientation.");

  TickType_t last_wake_time = xTaskGetTickCount();
  uint8_t orientation_count = 0;
  uint16_t sample_head = 0;
  uint32_t loop_counter = 0;
  bool daq_completed = false;

  const uint32_t required_stationary_samples = (CALIB_ACCEL_STATIONARY_HOLD_MS * TASK_CALIB_ACCEL_FREQ_HZ) / 1000;
  uint32_t stationary_counter = 0;
  Vec3<float> pose_ref{0.0f, 0.0f, 0.0f};
  bool has_pose_ref = false;
  bool motion_unlatched = true; // First orientation does not need prior unlatching
  Vec3<float> last_locked_pose{0.0f, 0.0f, 0.0f};

  // Phase 1: Interactive Data Acquisition across 6 orientations
  while(!daq_completed){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    loop_counter++;

    if(!accel_raw.empty()){
      Vec3<float> raw = accel_raw.front();

      if(!motion_unlatched){
        // Require pilot to move the drone away from the last locked pose
        Vec3<float> delta = raw - last_locked_pose;
        if(delta.norm() > CALIB_ACCEL_MOTION_UNLATCH_G){
          motion_unlatched = true;
          stationary_counter = 0;
          has_pose_ref = false;
          Serial.println("[CALIB_ACCEL] Motion detected. Awaiting next stationary orientation...");
        }
      }else{
        // Track stationary hold
        if(!has_pose_ref){
          pose_ref = raw;
          has_pose_ref = true;
          stationary_counter = 1;
        }else{
          bool within_tol = (fabsf(raw.x - pose_ref.x) < CALIB_ACCEL_STATIONARY_TOL_G) &&
                            (fabsf(raw.y - pose_ref.y) < CALIB_ACCEL_STATIONARY_TOL_G) &&
                            (fabsf(raw.z - pose_ref.z) < CALIB_ACCEL_STATIONARY_TOL_G);

          if(within_tol){
            stationary_counter++;
          }else{
            pose_ref = raw;
            stationary_counter = 1;
          }
        }

        // Check if stationary duration achieved
        if(stationary_counter >= required_stationary_samples){
          // Verify orientation divergence (>= 45 deg) against all previously locked poses
          bool is_diverged = true;
          for(uint8_t j = 0; j < orientation_count; ++j){
            float dot = pose_ref.dot(locked_orientations[j]);
            float norm_prod = pose_ref.norm() * locked_orientations[j].norm();
            if(norm_prod > 1e-4f){
              float cos_theta = dot / norm_prod;
              if(cos_theta > CALIB_ACCEL_MIN_DIVERGENCE_COS){
                is_diverged = false;
                break;
              }
            }
          }

          if(is_diverged){
            // Collect CALIB_ACCEL_SAMPLES_PER_ORIENTATION (50) samples directly
            Vec3<float> sum_samples{0.0f, 0.0f, 0.0f};
            uint8_t samples_collected = 0;

            while(samples_collected < CALIB_ACCEL_SAMPLES_PER_ORIENTATION){
              TickType_t s_now = xTaskGetTickCount();
              if((s_now - last_wake_time) > (period_ticks * 2)) last_wake_time = s_now;
              vTaskDelayUntil(&last_wake_time, period_ticks);

              if(!accel_raw.empty()){
                Vec3<float> s_sample = accel_raw.front();
                accel_cloud[sample_head + samples_collected] = s_sample;
                sum_samples += s_sample;
                samples_collected++;
              }
            }

            sample_head += CALIB_ACCEL_SAMPLES_PER_ORIENTATION;
            Vec3<float> centroid = sum_samples * (1.0f / static_cast<float>(CALIB_ACCEL_SAMPLES_PER_ORIENTATION));
            locked_orientations[orientation_count] = centroid;
            last_locked_pose = centroid;
            orientation_count++;

            Serial.printf("[CALIB_ACCEL] Orientation %u of %u locked! Pose: [%.3f, %.3f, %.3f] g\n",
                          orientation_count, CALIB_ACCEL_ORIENTATIONS_COUNT,
                          centroid.x, centroid.y, centroid.z);

            // Visual ACK: White flash burst
            CalibReportMsg lock_msg = {
              .target       = CALIB_TARGET_ACCEL,
              .status       = CALIB_STATUS_STEP_LOCKED,
              .current_step = orientation_count,
              .progress_pct = static_cast<uint8_t>((orientation_count * 100) / CALIB_ACCEL_ORIENTATIONS_COUNT),
              .timestamp_us = static_cast<uint32_t>(micros())
            };
            xQueueSend(calib_report_queue, &lock_msg, 0);

            if(orientation_count >= CALIB_ACCEL_ORIENTATIONS_COUNT){
              daq_completed = true;
            }else{
              // Small delay to let White flash burst start, then switch back to breathing Orange
              vTaskDelay(pdMS_TO_TICKS(100));
              CalibReportMsg resume_msg = {
                .target       = CALIB_TARGET_ACCEL,
                .status       = CALIB_STATUS_SAMPLING,
                .current_step = orientation_count,
                .progress_pct = static_cast<uint8_t>((orientation_count * 100) / CALIB_ACCEL_ORIENTATIONS_COUNT),
                .timestamp_us = static_cast<uint32_t>(micros())
              };
              xQueueSend(calib_report_queue, &resume_msg, 0);

              motion_unlatched = false;
              stationary_counter = 0;
              has_pose_ref = false;
              Serial.println("[CALIB_ACCEL] Rotate drone to next orientation...");
            }
          }else{
            // Pose is stationary but too close to an existing locked orientation
            if(loop_counter % 100 == 0)
              Serial.println("[CALIB_ACCEL] Pose already recorded or < 45 deg separation. Rotate to different face.");
          }
        }
      }
    }

    // Periodic healthcheck report at 10 Hz
    if(loop_counter % 20 == 0){
      CalibReportMsg msg = {
        .target       = CALIB_TARGET_ACCEL,
        .status       = CALIB_STATUS_HEALTHCHECK,
        .current_step = orientation_count,
        .progress_pct = static_cast<uint8_t>((orientation_count * 100) / CALIB_ACCEL_ORIENTATIONS_COUNT),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &msg, 0);
    }
  }

  // Phase 2: Iterative 6-Parameter Axis-Aligned Ellipsoid Fit (SIMD Adam)
  Serial.printf("[CALIB_ACCEL] Starting SIMD Adam optimization on %u points...\n", sample_head);

  CalibReportMsg comp_msg = {
    .target       = CALIB_TARGET_ACCEL,
    .status       = CALIB_STATUS_COMPUTING,
    .current_step = CALIB_ACCEL_ORIENTATIONS_COUNT,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &comp_msg, 0);

  Ellipsoid6ParamState fit_state;
  ellipsoid_fit_6param_init(fit_state, accel_cloud, CALIB_ACCEL_SAMPLE_COUNT,
                            CALIB_ACCEL_MAX_EPOCHS, CALIB_ACCEL_ADAM_LR, CALIB_ACCEL_ADAM_TOL,
                            CALIB_ACCEL_ORIENTATIONS_COUNT);

  Serial.printf("[CALIB_ACCEL] Initial RMSE: %.6f. Running %u epochs per cycle...\n",
                fit_state.initial_rmse, CALIB_ACCEL_EPOCHS_PER_CYCLE);

  bool opt_done = false;
  while(!opt_done){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    // Compute time-sliced Adam optimization epochs
    opt_done = ellipsoid_fit_6param_step(fit_state, accel_cloud, CALIB_ACCEL_SAMPLE_COUNT,
                                         CALIB_ACCEL_EPOCHS_PER_CYCLE, CALIB_ACCEL_ORIENTATIONS_COUNT);

    // Periodic progress report at 10 Hz
    if(fit_state.current_epoch % 10 == 0 || opt_done){
      CalibReportMsg solver_msg = {
        .target       = CALIB_TARGET_ACCEL,
        .status       = CALIB_STATUS_COMPUTING,
        .current_step = CALIB_ACCEL_ORIENTATIONS_COUNT,
        .progress_pct = static_cast<uint8_t>((fit_state.current_epoch * 100) / fit_state.max_epochs),
        .timestamp_us = static_cast<uint32_t>(micros())
      };
      xQueueSend(calib_report_queue, &solver_msg, 0);
    }
  }

  // Phase 3: Extract parameters, package 6-DOF model, and save to Flash NVS
  Ellipsoid6ParamResult params;
  ellipsoid_fit_6param_get_params(fit_state, params);

  Serial.printf("[CALIB_ACCEL] Optimization finished in %u epochs (converged=%s)!\n",
                params.epochs_run, params.converged ? "YES" : "NO");
  Serial.printf("[CALIB_ACCEL] Initial RMSE: %.6f -> Final RMSE: %.6f\n",
                params.initial_rmse, params.final_rmse);
  Serial.printf("[CALIB_ACCEL] Calibrated Model Parameters (6-DOF Axis-Aligned):\n");
  Serial.printf("  Zero-G Center Offset v0 (g): [%.5f, %.5f, %.5f]\n",
                params.v0.x, params.v0.y, params.v0.z);
  Serial.printf("  Sensitivity Scale Vector s (1/g): [%.5f, %.5f, %.5f]\n",
                params.s.x, params.s.y, params.s.z);

  // Store calibration parameters: s maps directly to 1.0g unit sphere
  calib_data.accel_bias  = params.v0;
  calib_data.accel_scale = params.s;

  bool saved_accel = calib_save_to_nvs();
  Serial.printf("[CALIB_ACCEL] Completed. Saved to NVS: %s\n", saved_accel ? "SUCCESS" : "FAILED");

  // Report final completion to FSM coordinator
  CalibReportMsg done_msg = {
    .target       = CALIB_TARGET_ACCEL,
    .status       = CALIB_STATUS_COMPLETED,
    .current_step = CALIB_ACCEL_ORIENTATIONS_COUNT,
    .progress_pct = 100,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &done_msg, portMAX_DELAY);

  Serial.println("[CALIB_ACCEL] Completed. Halting and awaiting FSM deletion...");
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

// Magnetometer Calibration: Continuous tumble rotation + SIMD Ellipsoid fit solver
void task_calib_mag(void* arg){
  // Point cloud buffer on task stack (reclaimed when FSM deletes task upon completion)
  Vec3<float> mag_cloud[CALIB_MAG_SAMPLE_COUNT];

  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_CALIB_MAG_FREQ_HZ);

  Serial.printf("[CALIB_MAG] Task started. Entering %u ms settling delay...\n",
                static_cast<unsigned int>(CALIB_SETTLING_DELAY_MS));

  // Report settling phase to FSM coordinator (LED: Fast-blinking Amber)
  CalibReportMsg settle_msg = {
    .target       = CALIB_TARGET_MAG,
    .status       = CALIB_STATUS_SETTLING,
    .current_step = 0,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &settle_msg, 0);

  vTaskDelay(pdMS_TO_TICKS(CALIB_SETTLING_DELAY_MS));

  // Report active sampling transition to FSM coordinator (LED: Breathing Magenta)
  CalibReportMsg sample_msg = {
    .target       = CALIB_TARGET_MAG,
    .status       = CALIB_STATUS_SAMPLING,
    .current_step = 0,
    .progress_pct = 0,
    .timestamp_us = static_cast<uint32_t>(micros())
  };
  xQueueSend(calib_report_queue, &sample_msg, 0);

  Serial.println("[CALIB_MAG] Settling complete. Rotate drone indefinitely across all 3D axes.");
  Serial.println("[CALIB_MAG] Press USER BUTTON to stop acquisition and begin optimization.");

  TickType_t last_wake_time = xTaskGetTickCount();
  uint16_t cloud_head = 0;
  uint16_t cloud_count = 0;
  uint32_t loop_counter = 0;
  Vec3<float> last_sample{0.0f, 0.0f, 0.0f};
  bool has_first_sample = false;
  bool daq_completed = false;

  // Phase 1: Real-time user continuous tumble rotation point-cloud acquisition
  while(!daq_completed){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    loop_counter++;

    // Ingest latest un-filtered raw measurement from ipc.h
    if(!mag_raw.empty()){
      Vec3<float> raw = mag_raw.front();
      if(!has_first_sample){
        mag_cloud[cloud_head] = raw;
        cloud_head = (cloud_head + 1) % CALIB_MAG_SAMPLE_COUNT;
        cloud_count = 1;
        last_sample = raw;
        has_first_sample = true;
      }else{
        Vec3<float> diff = raw - last_sample;
        float dist_sq = diff.norm_sq();
        if(dist_sq >= (CALIB_MAG_MIN_DIST_GAUSS * CALIB_MAG_MIN_DIST_GAUSS)){
          mag_cloud[cloud_head] = raw;
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
        .status       = CALIB_STATUS_HEALTHCHECK,
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

  Ellipsoid9ParamState fit_state;
  ellipsoid_fit_9param_init(fit_state, mag_cloud, cloud_count,
                            CALIB_MAG_MAX_EPOCHS, CALIB_MAG_ADAM_LR, CALIB_MAG_ADAM_TOL, CALIB_MAG_MIN_SAMPLES);

  Serial.printf("[CALIB_MAG] Initial RMSE: %.6f. Running %u epochs per cycle...\n",
                fit_state.initial_rmse, CALIB_MAG_EPOCHS_PER_CYCLE);

  bool opt_done = false;
  while(!opt_done){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    // Compute time-sliced Adam optimization epochs
    opt_done = ellipsoid_fit_9param_step(fit_state, mag_cloud, cloud_count,
                                         CALIB_MAG_EPOCHS_PER_CYCLE, CALIB_MAG_MIN_SAMPLES);

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
  }

  // Phase 3: Extract parameters, package unit sphere matrix, and save to Flash NVS
  Ellipsoid9ParamResult params;
  ellipsoid_fit_9param_get_params(fit_state, params);

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
  Serial.printf("  Geomagnetic Magnitude B0: %.5f Gauss\n", params.b0);

  // Store calibration parameters: W maps directly to unit sphere for Extended Kalman Filter
  calib_data.mag_bias      = params.v0;
  calib_data.mag_soft_iron = params.W;
  calib_data.mag_field_ref = params.b0;

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
