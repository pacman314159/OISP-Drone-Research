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
      uint32_t ts_snap[IMU_RAW_RING_BUF_SIZE];
      Vec3<float> gyro_snap[IMU_RAW_RING_BUF_SIZE];
      Vec3<float> accel_snap[IMU_RAW_RING_BUF_SIZE];

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
      uint32_t ts_snap[IMU_FILT_RING_BUF_SIZE];
      Vec3<float> gyro_snap[IMU_FILT_RING_BUF_SIZE];
      Vec3<float> accel_snap[IMU_FILT_RING_BUF_SIZE];

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
      uint32_t ts_snap[IMU_RAW_RING_BUF_SIZE];
      Vec3<float> mag_raw_snap[IMU_RAW_RING_BUF_SIZE];

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
      uint32_t ts_snap[IMU_FILT_RING_BUF_SIZE];
      Vec3<float> mag_filt_snap[IMU_FILT_RING_BUF_SIZE];

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
      uint32_t ts_snap[IMU_RAW_RING_BUF_SIZE];
      float baro_raw_snap[IMU_RAW_RING_BUF_SIZE];

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
      uint32_t ts_snap[IMU_FILT_RING_BUF_SIZE];
      float baro_filt_snap[IMU_FILT_RING_BUF_SIZE];

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
