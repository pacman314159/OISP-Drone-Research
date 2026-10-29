#include "sd_logger_task.h"
#include <Arduino.h>
#include <cstring>
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/sysview_tracing.h"
#include "drivers/storage/sd_card_driver.h"

volatile bool sd_stop_logging_requested = false;

struct __attribute__((packed)) PktImuRaw {
  uint8_t     type;
  uint32_t    timestamp_us;
  uint8_t     _pad[3];
  Vec3<float> gyro;
  Vec3<float> accel;
};
static_assert(sizeof(PktImuRaw) == 32, "PktImuRaw layout must be strictly 32 bytes aligned");

struct __attribute__((packed)) PktImuFilt {
  uint8_t     type;
  uint32_t    timestamp_us;
  uint8_t     _pad[3];
  Vec3<float> gyro;
  Vec3<float> accel;
};
static_assert(sizeof(PktImuFilt) == 32, "PktImuFilt layout must be strictly 32 bytes aligned");

struct __attribute__((packed)) PktMagRaw {
  uint8_t     type;
  uint32_t    timestamp_us;
  uint8_t     _pad[3];
  Vec3<float> mag;
};
static_assert(sizeof(PktMagRaw) == 20, "PktMagRaw layout must be strictly 20 bytes aligned");

struct __attribute__((packed)) PktMagFilt {
  uint8_t     type;
  uint32_t    timestamp_us;
  uint8_t     _pad[3];
  Vec3<float> mag;
};
static_assert(sizeof(PktMagFilt) == 20, "PktMagFilt layout must be strictly 20 bytes aligned");

struct __attribute__((packed)) PktBaroRaw {
  uint8_t  type;
  uint32_t timestamp_us;
  float    pressure;
};
static_assert(sizeof(PktBaroRaw) == 9, "PktBaroRaw layout must be strictly 9 bytes packed");

struct __attribute__((packed)) PktBaroFilt {
  uint8_t  type;
  uint32_t timestamp_us;
  float    pressure;
};
static_assert(sizeof(PktBaroFilt) == 9, "PktBaroFilt layout must be strictly 9 bytes packed");

struct __attribute__((packed)) PktAttQuat {
  uint8_t  type;
  uint32_t timestamp_us;
  uint8_t  _pad[3];
  float    q[4];
};
static_assert(sizeof(PktAttQuat) == 24, "PktAttQuat layout must be strictly 24 bytes aligned");

struct __attribute__((packed)) PktAttEuler {
  uint8_t     type;
  uint32_t    timestamp_us;
  uint8_t     _pad[3];
  Vec3<float> euler;
};
static_assert(sizeof(PktAttEuler) == 20, "PktAttEuler layout must be strictly 20 bytes aligned");

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

struct __attribute__((packed)) PktAlt {
  uint8_t     type;
  uint32_t    timestamp_us;
  uint8_t     _pad[3];
  Vec3<float> state; // z (m), v_z (m/s), a_z_inertial (m/s^2)
};
static_assert(sizeof(PktAlt) == 20, "PktAlt layout must be strictly 20 bytes aligned");

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

template<typename TPkt, typename TData, size_t N, typename Builder>
inline void log_single_stream(RingBuffer<uint32_t, N>& ts_ring,
                              RingBuffer<TData, N>& data_ring,
                              ChannelTracker& tracker,
                              uint8_t* sector_buf,
                              size_t& buf_index,
                              SDCardDriver* sd_driver,
                              TickType_t& last_flush_tick,
                              Builder builder){
  uint32_t ts_snap[N];
  TData data_snap[N];
  size_t count = ts_ring.get_snapshot(ts_snap);
  size_t data_count = data_ring.get_snapshot(data_snap);
  if(data_count < count) count = data_count;

  for(size_t i = 0; i < count; ++i){
    uint32_t curr_ts = ts_snap[i];
    if(tracker.should_log(curr_ts)){
      TPkt pkt = builder(curr_ts, data_snap[i]);
      append_packet(&pkt, sizeof(TPkt), sector_buf, buf_index, sd_driver, last_flush_tick);
    }
  }
}

template<typename TPkt, typename TData1, typename TData2, size_t N, typename Builder>
inline void log_dual_stream(RingBuffer<uint32_t, N>& ts_ring,
                            RingBuffer<TData1, N>& data1_ring,
                            RingBuffer<TData2, N>& data2_ring,
                            ChannelTracker& tracker,
                            uint8_t* sector_buf,
                            size_t& buf_index,
                            SDCardDriver* sd_driver,
                            TickType_t& last_flush_tick,
                            Builder builder){
  uint32_t ts_snap[N];
  TData1 data1_snap[N];
  TData2 data2_snap[N];
  size_t count = ts_ring.get_snapshot(ts_snap);
  size_t d1_count = data1_ring.get_snapshot(data1_snap);
  size_t d2_count = data2_ring.get_snapshot(data2_snap);
  if(d1_count < count) count = d1_count;
  if(d2_count < count) count = d2_count;

  for(size_t i = 0; i < count; ++i){
    uint32_t curr_ts = ts_snap[i];
    if(tracker.should_log(curr_ts)){
      TPkt pkt = builder(curr_ts, data1_snap[i], data2_snap[i]);
      append_packet(&pkt, sizeof(TPkt), sector_buf, buf_index, sd_driver, last_flush_tick);
    }
  }
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
  ChannelTracker trk_alt;

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
      log_dual_stream<PktImuRaw>(raw_accel_gyro_timestamp, gyro_raw, accel_raw, trk_imu_raw,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const Vec3<float>& g, const Vec3<float>& a){
          return PktImuRaw{PKT_TYPE_IMU_RAW, ts, {0, 0, 0}, g, a};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_IMU_FILT) != 0){
      log_dual_stream<PktImuFilt>(filtered_accel_gyro_timestamp, gyro, accel, trk_imu_filt,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const Vec3<float>& g, const Vec3<float>& a){
          return PktImuFilt{PKT_TYPE_IMU_FILT, ts, {0, 0, 0}, g, a};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_MAG_RAW) != 0){
      log_single_stream<PktMagRaw>(raw_mag_timestamp, mag_raw, trk_mag_raw,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const Vec3<float>& m){
          return PktMagRaw{PKT_TYPE_MAG_RAW, ts, {0, 0, 0}, m};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_MAG_FILT) != 0){
      log_single_stream<PktMagFilt>(filtered_mag_timestamp, mag, trk_mag_filt,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const Vec3<float>& m){
          return PktMagFilt{PKT_TYPE_MAG_FILT, ts, {0, 0, 0}, m};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_BARO_RAW) != 0){
      log_single_stream<PktBaroRaw>(pressure_timestamp, pressure, trk_baro_raw,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, float p){
          return PktBaroRaw{PKT_TYPE_BARO_RAW, ts, p};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_BARO_FILT) != 0){
      log_single_stream<PktBaroFilt>(pressure_timestamp, pressure, trk_baro_filt,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, float p){
          return PktBaroFilt{PKT_TYPE_BARO_FILT, ts, p};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_POWER) != 0){
      log_single_stream<PktPower>(power_timestamp, power_data, trk_power,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const PowerMeasurement& pw){
          return PktPower{PKT_TYPE_POWER, ts, {0, 0, 0}, pw.voltage_mv, pw.current_ma, pw.power_mw, pw.energy_mah};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_RC) != 0){
      log_single_stream<PktRc>(rc_timestamp, rc_data, trk_rc,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const RcChannels& rc){
          PktRc pkt;
          pkt.type = PKT_TYPE_RC;
          pkt.timestamp_us = ts;
          pkt._pad[0] = 0; pkt._pad[1] = 0; pkt._pad[2] = 0;
          for(uint8_t ch_idx = 0; ch_idx < 6; ++ch_idx){
            pkt.channels[ch_idx] = (ch_idx < rc.channel_count)
              ? static_cast<float>(rc.channels[ch_idx])
              : 1500.0f;
          }
          return pkt;
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_ATT_QUAT) != 0){
      log_single_stream<PktAttQuat>(att_timestamp, att_quat, trk_att_quat,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const Vec4& q){
          return PktAttQuat{PKT_TYPE_ATT_QUAT, ts, {0, 0, 0}, {q[0], q[1], q[2], q[3]}};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_ATT_EULER) != 0){
      log_single_stream<PktAttEuler>(att_timestamp, att_euler, trk_att_euler,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const Vec3<float>& eu){
          return PktAttEuler{PKT_TYPE_ATT_EULER, ts, {0, 0, 0}, eu};
        });
    }

    if constexpr((ACTIVE_SD_LOG_PACKETS_MASK & LOG_MASK_ALT) != 0){
      log_single_stream<PktAlt>(alt_timestamp, alt_state, trk_alt,
        sector_buf, buf_index, sd_driver, last_flush_tick,
        [](uint32_t ts, const Vec3<float>& alt){
          return PktAlt{PKT_TYPE_ALT, ts, {0, 0, 0}, alt};
        });
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
