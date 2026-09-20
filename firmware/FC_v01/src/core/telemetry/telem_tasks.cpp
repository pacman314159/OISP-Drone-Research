#include <Arduino.h>
#include "telem_tasks.h"
#include "config.h"
#include "middleware/ipc.h"
#include "drivers/telemetry/ble_driver.h"
#include "drivers/telemetry/ble_packet.h"

void ble_vec3_trans_task(void* arg){
  TelemSensorSource sensor_src = TELEM_SENSOR_GYRO;
  if(arg != nullptr){
    const TelemSensorSource* src_ptr = static_cast<const TelemSensorSource*>(arg);
    switch(*src_ptr){
      case TELEM_SENSOR_ACCEL:
        sensor_src = TELEM_SENSOR_ACCEL;
        break;
      case TELEM_SENSOR_MAG:
        sensor_src = TELEM_SENSOR_MAG;
        break;
      case TELEM_SENSOR_GYRO:
      default:
        sensor_src = TELEM_SENSOR_GYRO;
        break;
    }
  }

  uint32_t ts_snapshot[IMU_RAW_RING_BUF_SIZE];
  Vec3<float> vec3_snapshot[IMU_RAW_RING_BUF_SIZE];
  BleVec3Packet<float> ble_payload_buf[BLE_TELEM_SAMPLES_PER_PACKET];
  size_t ble_payload_count = 0;
  uint32_t last_ingested_ts = 0;

  TickType_t last_wake_time = xTaskGetTickCount();
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_BLE_VEC3_TRANS_FREQ_HZ);

  while(true){
    vTaskDelayUntil(&last_wake_time, period_ticks);

    bool ts_ok = accel_gyro_timestamp.get_snapshot(ts_snapshot);
    bool vec3_ok = false;
    size_t sample_count = 0;

    switch(sensor_src){
      case TELEM_SENSOR_ACCEL:
        vec3_ok = accel_raw.get_snapshot(vec3_snapshot);
        sample_count = accel_raw.count();
        break;
      case TELEM_SENSOR_MAG:
        vec3_ok = mag_raw.get_snapshot(vec3_snapshot);
        sample_count = mag_raw.count();
        break;
      case TELEM_SENSOR_GYRO:
      default:
        vec3_ok = gyro_raw.get_snapshot(vec3_snapshot);
        sample_count = gyro_raw.count();
        break;
    }

    if(ts_ok && vec3_ok){
      for(size_t i = 0; i < sample_count; ++i){
        uint32_t current_ts = ts_snapshot[i];
        if(last_ingested_ts == 0 || static_cast<int32_t>(current_ts - last_ingested_ts) > 0){
          uint32_t ts_to_send = current_ts;
          if(BLE_TELEM_TIMESTAMP_UNIT == TIMESTAMP_UNIT_MILLISECONDS)
            ts_to_send = current_ts / 1000;

          ble_payload_buf[ble_payload_count++] = BleVec3Packet<float>(ts_to_send, vec3_snapshot[i]);
          last_ingested_ts = current_ts;

          if(ble_payload_count >= BLE_TELEM_SAMPLES_PER_PACKET){
            if(is_ble_connected())
              send_gyro_telemetry_notification(reinterpret_cast<const uint8_t*>(ble_payload_buf), ble_payload_count * sizeof(BleVec3Packet<float>));
            ble_payload_count = 0;
          }
        }
      }
    }

  }
}
