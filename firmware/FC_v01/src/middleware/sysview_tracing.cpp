#include "sysview_tracing.h"

#if (defined(CONFIG_SYSVIEW_ENABLE) && CONFIG_SYSVIEW_ENABLE) || (defined(CONFIG_SEGGER_SYSVIEW_ENABLE) && CONFIG_SEGGER_SYSVIEW_ENABLE) || (defined(CONFIG_APPTRACE_SV_ENABLE) && CONFIG_APPTRACE_SV_ENABLE)

// Marker resource table: one row per TaskIDs entry.
// The static_assert below turns "added a task but forgot to name its marker"
// into a compile error instead of a silently unnamed trace channel.
struct MarkerName {
  uint32_t    id;
  const char* name;
};

static constexpr MarkerName k_marker_names[] = {
  {TASK_ACCEL_GYRO_DAQ_ID,        TASK_ACCEL_GYRO_DAQ_NAME},
  {TASK_ACCEL_GYRO_PREPROCESS_ID, TASK_ACCEL_GYRO_PREPROCESS_NAME},
  {TASK_BARO_DAQ_ID,              TASK_BARO_DAQ_NAME},
  {TASK_MAG_DAQ_ID,               TASK_MAG_DAQ_NAME},
  {TASK_MAG_PREPROCESS_ID,        TASK_MAG_PREPROCESS_NAME},
  {TASK_BLE_TELEM_ID,             TASK_BLE_TELEM_NAME},
  {TASK_SD_LOGGER_ID,             TASK_SD_LOGGER_NAME},
  {TASK_BLE_OTA_ID,               TASK_BLE_OTA_NAME},
  {TASK_FSM_ID,                   TASK_FSM_NAME},
  {TASK_STATUS_LED_ID,            TASK_STATUS_LED_NAME},
  {TASK_CALIB_GYRO_ID,            TASK_CALIB_GYRO_NAME},
  {TASK_CALIB_ACCEL_ID,           TASK_CALIB_ACCEL_NAME},
  {TASK_CALIB_MAG_ID,             TASK_CALIB_MAG_NAME},
  {TASK_CALIB_BARO_ID,            TASK_CALIB_BARO_NAME},
  {TASK_RC_RECEIVER_ID,           TASK_RC_RECEIVER_NAME},
  {TASK_EKF_ROLL_PITCH_ID,        TASK_EKF_ROLL_PITCH_NAME},
  {TASK_EKF_YAW_ID,               TASK_EKF_YAW_NAME},
  {TASK_EKF_ALTITUDE_ID,          TASK_EKF_ALTITUDE_NAME},
  {TASK_MOTOR_ID,                 TASK_MOTOR_NAME}
};

static_assert(sizeof(k_marker_names) / sizeof(k_marker_names[0]) == TASK_COUNT,
              "Every TaskIDs entry must have a SystemView marker name (note 0007 §9).");

void sysview_init_markers(){
  for(const MarkerName& m : k_marker_names){
    // Resource ID == task_id + 1, matching SYSVIEW_START()/SYSVIEW_END().
    SEGGER_SYSVIEW_NameResource(m.id + 1, m.name);
  }
}

#endif
