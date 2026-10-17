#include "sd_logger_task.h"
#include <Arduino.h>
#include <cstring>
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/sysview_tracing.h"
#include "drivers/storage/sd_card_driver.h"

struct __attribute__((packed)) PktImuRaw {
  uint8_t     type;
  uint32_t    timestamp_us;
  Vec3<float> gyro;
  Vec3<float> accel;
};

struct __attribute__((packed)) PktImuFilt {
  uint8_t     type;
  uint32_t    timestamp_us;
  Vec3<float> gyro;
  Vec3<float> accel;
};

struct __attribute__((packed)) PktMagRaw {
  uint8_t     type;
  uint32_t    timestamp_us;
  Vec3<float> mag;
};

struct __attribute__((packed)) PktMagFilt {
  uint8_t     type;
  uint32_t    timestamp_us;
  Vec3<float> mag;
};

struct __attribute__((packed)) PktBaroRaw {
  uint8_t  type;
  uint32_t timestamp_us;
  float    pressure;
};

struct __attribute__((packed)) PktBaroFilt {
  uint8_t  type;
  uint32_t timestamp_us;
  float    pressure;
};

struct __attribute__((packed)) PktAttQuat {
  uint8_t  type;
  uint32_t timestamp_us;
  float    q[4];
};

struct __attribute__((packed)) PktAttEuler {
  uint8_t     type;
  uint32_t    timestamp_us;
  Vec3<float> euler;
};

struct __attribute__((packed)) PktPower {
  uint8_t  type;
  uint32_t timestamp_us;
  uint8_t  _pad[3];
  float    voltage_mv;
  float    current_ma;
  float    power_mw;
  float    energy_mah;
};

struct __attribute__((packed)) PktRc {
  uint8_t  type;
  uint32_t timestamp_us;
  uint8_t  _pad[3];
  float    channels[6];
};
static_assert(sizeof(PktRc) == 32, "PktRc layout must be strictly 32 bytes aligned");

struct ChannelTracker {
  uint32_t last_ts    = 0;
  bool     has_logged = false;

  bool should_log(uint32_t curr_ts){
    if(!has_logged || static_cast<int32_t>(curr_ts - last_ts) > 0){
      last_ts = curr_ts;
      has_logged = true;
      return true;
    }
    return false;
  }
};

static void append_packet(const void* data, size_t len, uint8_t* sector_buf, size_t& buf_index, SDCardDriver* sd_driver, TickType_t& last_flush_tick){
  if(buf_index + len > 512){
    memset(&sector_buf[buf_index], 0, 512 - buf_index);
    if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
      sd_driver->write_sector(sector_buf, 512);
      xSemaphoreGive(spi_bus_mutex);
    }
    buf_index = 0;
    last_flush_tick = xTaskGetTickCount();
  }
  memcpy(&sector_buf[buf_index], data, len);
  buf_index += len;
}

void sd_logger_task(void* arg){
  if(arg == nullptr) return;

  SDCardDriver* sd_driver = static_cast<SDCardDriver*>(arg);

  if constexpr(ACTIVE_SD_LOG_PACKETS_MASK != LOG_MASK_NONE){
    if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
      if(sd_driver->init()){
        char filename[32];
        uint16_t session_id = 0;
        if(sd_driver->open_next_log_file(filename, sizeof(filename), session_id)){
          LogFileHeader header = {};
          memcpy(header.magic, "FCLG", 4);
          header.session_id              = session_id;
          strncpy(header.board_name, BOARD_IDENTIFIER, sizeof(header.board_name) - 1);
          header.boot_timestamp_us       = static_cast<uint32_t>(micros());
          header.registered_packets_mask = ACTIVE_SD_LOG_PACKETS_MASK;
          header.limit_policy            = static_cast<uint8_t>(LOG_LIMIT_POLICY);
          header.max_file_size_bytes     = LOG_MAX_FILE_SIZE_BYTES;
          header.circular_head_sector    = 1;
          sd_driver->write_header(header);
          Serial.printf("[SD_LOGGER] Log file opened successfully: %s (Session #%u)\n", filename, session_id);
        }
      }
      xSemaphoreGive(spi_bus_mutex);
    }
  }

  if(!sd_driver->is_open()){
    Serial.println("[SD_LOGGER] SD card not mounted or log file failed to open. Task suspending.");
    vTaskSuspend(nullptr);
    return;
  }

  const uint32_t task_id = TASK_SD_LOGGER_ID;

  alignas(4) uint8_t sector_buf[512];
  size_t buf_index = 0;

  TickType_t last_wake_time = xTaskGetTickCount();
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_SD_LOGGER_FREQ_HZ);
  const TickType_t max_flush_ticks = pdMS_TO_TICKS(LOG_SECTOR_TIMEOUT_MS);
  TickType_t last_flush_tick = xTaskGetTickCount();
  const TickType_t sync_period_ticks = pdMS_TO_TICKS(LOG_METADATA_SYNC_PERIOD_MS);
  TickType_t last_sync_tick = xTaskGetTickCount();

  ChannelTracker trk_imu_raw;
  ChannelTracker trk_imu_filt;
  ChannelTracker trk_mag_raw;
  ChannelTracker trk_mag_filt;
  ChannelTracker trk_baro_raw;
  ChannelTracker trk_baro_filt;
  ChannelTracker trk_power;
  ChannelTracker trk_rc;
  ChannelTracker trk_att_quat;
  ChannelTracker trk_att_euler;

  while(true){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2))
      last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(sd_stop_logging_requested){
      if(buf_index > 0){
        memset(&sector_buf[buf_index], 0, 512 - buf_index);
        if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
          sd_driver->write_sector(sector_buf, 512);
          xSemaphoreGive(spi_bus_mutex);
        }
        buf_index = 0;
      }
      if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
        sd_driver->close();
        xSemaphoreGive(spi_bus_mutex);
      }
      Serial.println("[SD_LOGGER] Log closed cleanly upon user request. Task suspending.");
      SYSVIEW_END(task_id);
      vTaskSuspend(nullptr);
    }

    SYSVIEW_START(task_id);

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_IMU_RAW) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      Vec3<float> gyro_snap[RING_BUF_SIZE];
      Vec3<float> accel_snap[RING_BUF_SIZE];

      size_t count = raw_accel_gyro_timestamp.get_snapshot(ts_snap);
      size_t gyro_count = gyro_raw.get_snapshot(gyro_snap);
      size_t accel_count = accel_raw.get_snapshot(accel_snap);
      if(gyro_count < count) count = gyro_count;
      if(accel_count < count) count = accel_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_imu_raw.should_log(curr_ts)){
          PktImuRaw pkt;
          pkt.type         = PKT_TYPE_IMU_RAW;
          pkt.timestamp_us = curr_ts;
          pkt.gyro         = gyro_snap[i];
          pkt.accel        = accel_snap[i];
          append_packet(&pkt, sizeof(PktImuRaw), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_IMU_FILT) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      Vec3<float> gyro_snap[RING_BUF_SIZE];
      Vec3<float> accel_snap[RING_BUF_SIZE];

      size_t count = filtered_accel_gyro_timestamp.get_snapshot(ts_snap);
      size_t gyro_count = gyro.get_snapshot(gyro_snap);
      size_t accel_count = accel.get_snapshot(accel_snap);
      if(gyro_count < count) count = gyro_count;
      if(accel_count < count) count = accel_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_imu_filt.should_log(curr_ts)){
          PktImuFilt pkt;
          pkt.type         = PKT_TYPE_IMU_FILT;
          pkt.timestamp_us = curr_ts;
          pkt.gyro         = gyro_snap[i];
          pkt.accel        = accel_snap[i];
          append_packet(&pkt, sizeof(PktImuFilt), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_MAG_RAW) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      Vec3<float> mag_raw_snap[RING_BUF_SIZE];

      size_t count = raw_mag_timestamp.get_snapshot(ts_snap);
      size_t raw_count = mag_raw.get_snapshot(mag_raw_snap);
      if(raw_count < count) count = raw_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_mag_raw.should_log(curr_ts)){
          PktMagRaw pkt;
          pkt.type         = PKT_TYPE_MAG_RAW;
          pkt.timestamp_us = curr_ts;
          pkt.mag          = mag_raw_snap[i];
          append_packet(&pkt, sizeof(PktMagRaw), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_MAG_FILT) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      Vec3<float> mag_filt_snap[RING_BUF_SIZE];

      size_t count = filtered_mag_timestamp.get_snapshot(ts_snap);
      size_t filt_count = mag.get_snapshot(mag_filt_snap);
      if(filt_count < count) count = filt_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_mag_filt.should_log(curr_ts)){
          PktMagFilt pkt;
          pkt.type         = PKT_TYPE_MAG_FILT;
          pkt.timestamp_us = curr_ts;
          pkt.mag          = mag_filt_snap[i];
          append_packet(&pkt, sizeof(PktMagFilt), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_BARO_RAW) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      float baro_raw_snap[RING_BUF_SIZE];

      size_t count = pressure_timestamp.get_snapshot(ts_snap);
      size_t raw_count = pressure.get_snapshot(baro_raw_snap);
      if(raw_count < count) count = raw_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_baro_raw.should_log(curr_ts)){
          PktBaroRaw pkt;
          pkt.type         = PKT_TYPE_BARO_RAW;
          pkt.timestamp_us = curr_ts;
          pkt.pressure     = baro_raw_snap[i];
          append_packet(&pkt, sizeof(PktBaroRaw), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_BARO_FILT) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      float baro_filt_snap[RING_BUF_SIZE];

      size_t count = pressure_timestamp.get_snapshot(ts_snap);
      size_t filt_count = pressure.get_snapshot(baro_filt_snap);
      if(filt_count < count) count = filt_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_baro_filt.should_log(curr_ts)){
          PktBaroFilt pkt;
          pkt.type         = PKT_TYPE_BARO_FILT;
          pkt.timestamp_us = curr_ts;
          pkt.pressure     = baro_filt_snap[i];
          append_packet(&pkt, sizeof(PktBaroFilt), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_POWER) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      PowerMeasurement power_snap[RING_BUF_SIZE];

      size_t count = power_timestamp.get_snapshot(ts_snap);
      size_t raw_count = power_data.get_snapshot(power_snap);
      if(raw_count < count) count = raw_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_power.should_log(curr_ts)){
          PktPower pkt;
          pkt.type         = PKT_TYPE_POWER;
          pkt.timestamp_us = curr_ts;
          pkt._pad[0] = 0; pkt._pad[1] = 0; pkt._pad[2] = 0;
          pkt.voltage_mv   = power_snap[i].voltage_mv;
          pkt.current_ma   = power_snap[i].current_ma;
          pkt.power_mw     = power_snap[i].power_mw;
          pkt.energy_mah   = power_snap[i].energy_mah;
          append_packet(&pkt, sizeof(PktPower), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_RC) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      RcChannels rc_snap[RING_BUF_SIZE];

      size_t count = rc_timestamp.get_snapshot(ts_snap);
      size_t raw_count = rc_data.get_snapshot(rc_snap);
      if(raw_count < count) count = raw_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_rc.should_log(curr_ts)){
          PktRc pkt;
          pkt.type         = PKT_TYPE_RC;
          pkt.timestamp_us = curr_ts;
          pkt._pad[0] = 0; pkt._pad[1] = 0; pkt._pad[2] = 0;
          for(uint8_t ch_idx = 0; ch_idx < 6; ++ch_idx){
            pkt.channels[ch_idx] = (ch_idx < rc_snap[i].channel_count)
              ? static_cast<float>(rc_snap[i].channels[ch_idx])
              : 1500.0f;
          }
          append_packet(&pkt, sizeof(PktRc), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_ATT_QUAT) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      Vec4 quat_snap[RING_BUF_SIZE];

      size_t count = att_timestamp.get_snapshot(ts_snap);
      size_t quat_count = att_quat.get_snapshot(quat_snap);
      if(quat_count < count) count = quat_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_att_quat.should_log(curr_ts)){
          PktAttQuat pkt;
          pkt.type         = PKT_TYPE_ATT_QUAT;
          pkt.timestamp_us = curr_ts;
          pkt.q[0]         = quat_snap[i][0];
          pkt.q[1]         = quat_snap[i][1];
          pkt.q[2]         = quat_snap[i][2];
          pkt.q[3]         = quat_snap[i][3];
          append_packet(&pkt, sizeof(PktAttQuat), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_ATT_EULER) != 0){
      uint32_t ts_snap[RING_BUF_SIZE];
      Vec3<float> euler_snap[RING_BUF_SIZE];

      size_t count = att_timestamp.get_snapshot(ts_snap);
      size_t euler_count = att_euler.get_snapshot(euler_snap);
      if(euler_count < count) count = euler_count;

      for(size_t i = 0; i < count; ++i){
        uint32_t curr_ts = ts_snap[i];
        if(trk_att_euler.should_log(curr_ts)){
          PktAttEuler pkt;
          pkt.type         = PKT_TYPE_ATT_EULER;
          pkt.timestamp_us = curr_ts;
          pkt.euler        = euler_snap[i];
          append_packet(&pkt, sizeof(PktAttEuler), sector_buf, buf_index, sd_driver, last_flush_tick);
        }
      }
    }

    TickType_t now_ticks = xTaskGetTickCount();
    if(buf_index > 0 && (now_ticks - last_flush_tick) >= max_flush_ticks){
      memset(&sector_buf[buf_index], 0, 512 - buf_index);
      if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
        sd_driver->write_sector(sector_buf, 512);
        sd_driver->flush();
        xSemaphoreGive(spi_bus_mutex);
      }
      buf_index = 0;
      last_flush_tick = now_ticks;
    }

    if((now_ticks - last_sync_tick) >= sync_period_ticks){
      if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
        sd_driver->flush();
        xSemaphoreGive(spi_bus_mutex);
      }
      last_sync_tick = now_ticks;
    }

    if(sd_driver->get_bytes_written() >= LOG_MAX_FILE_SIZE_BYTES){
      if(LOG_LIMIT_POLICY == LOG_LIMIT_STOP_ON_SIZE){
        if(buf_index > 0){
          memset(&sector_buf[buf_index], 0, 512 - buf_index);
          if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
            sd_driver->write_sector(sector_buf, 512);
            xSemaphoreGive(spi_bus_mutex);
          }
          buf_index = 0;
        }
        if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
          sd_driver->close();
          xSemaphoreGive(spi_bus_mutex);
        }
        SYSVIEW_END(task_id);
        vTaskSuspend(nullptr);
      }else if(LOG_LIMIT_POLICY == LOG_LIMIT_CIRCULAR_OVERWRITE){
        if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
          sd_driver->reset_file_pointer_to_data_start();
          xSemaphoreGive(spi_bus_mutex);
        }
      }
    }

    SYSVIEW_END(task_id);
  }
}
