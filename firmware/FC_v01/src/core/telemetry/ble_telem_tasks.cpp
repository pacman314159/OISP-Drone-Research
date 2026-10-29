#include <Arduino.h>
#include "ble_telem_tasks.h"
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/critical_section.h"
#include "middleware/sysview_tracing.h"
#include "drivers/telemetry/ble_driver.h"
#include "drivers/telemetry/ble_packet.h"

// =============================================================================
// GENERIC TELEMETRY CHANNEL SERIALIZER & DUAL-TRIGGER FLUSHER
// =============================================================================
// Adheres strictly to FCDatMon Stride (S = 4 + 4*N bytes) per Note 0011 and
// Dual-Trigger Flushing Contract (Capacity vs. 100ms Timeout) per Note 0010.
// =============================================================================

template<typename TPacket, size_t MAX_SAMPLES>
class BleTelemetryChannel {
public:
  explicit BleTelemetryChannel(uint16_t mask_bit)
    : _mask_bit(mask_bit), _payload_count(0), _last_ingested_ts(0), _last_send_tick(0){}

  template<typename TData, size_t N>
  void process(RingBuffer<TData, N>& data_ring, RingBuffer<uint32_t, N>& ts_ring, TickType_t now_tick, TickType_t timeout_ticks){
    // Zero-overhead subscription gating: Skip if FCDatMon is not subscribed to this characteristic
    if(!is_ble_stream_subscribed(_mask_bit)) return;

    TData data_snap[N];
    uint32_t ts_snap[N]; // us

    size_t count_data = data_ring.get_snapshot(data_snap);
    size_t count_ts   = ts_ring.get_snapshot(ts_snap);
    size_t count      = (count_data < count_ts) ? count_data : count_ts;

    for(size_t i = 0; i < count; ++i){
      uint32_t cur_ts = ts_snap[i]; // us
      if(_last_ingested_ts == 0 || static_cast<int32_t>(cur_ts - _last_ingested_ts) > 0){
        uint32_t ts_to_send = cur_ts; // us
        if(BLE_TELEM_TIMESTAMP_UNIT == TIMESTAMP_UNIT_MILLISECONDS)
          ts_to_send = cur_ts / 1000; // ms

        _payload_buf[_payload_count++] = TPacket(ts_to_send, data_snap[i]);
        _last_ingested_ts = cur_ts;

        // Capacity Trigger: Flush full batch once max sample threshold reached (MTU optimization)
        if(_payload_count >= MAX_SAMPLES)
          flush(now_tick);
      }
    }

    // Timeout Trigger: Flush partial batch if countdown timer expired (prevents plot lag)
    if(_payload_count > 0 && (now_tick - _last_send_tick) >= timeout_ticks)
      flush(now_tick);
  }

  void flush(TickType_t now_tick){
    if(_payload_count > 0){
      send_ble_stream_notification(_mask_bit, reinterpret_cast<const uint8_t*>(_payload_buf), _payload_count * sizeof(TPacket));
      _payload_count = 0;
      _last_send_tick = now_tick;
    }
  }

private:
  uint16_t   _mask_bit;
  TPacket    _payload_buf[MAX_SAMPLES];
  size_t     _payload_count;
  uint32_t   _last_ingested_ts; // us
  TickType_t _last_send_tick;  // FreeRTOS ticks
};

// Static channel instances allocated in BSS to prevent stack overflow
static BleTelemetryChannel<BleGyroPacket, BLE_GYRO_SAMPLES_PER_PACKET>
  s_ch_gyro_raw(BLE_TELEM_MASK_GYRO_RAW);
static BleTelemetryChannel<BleGyroPacket, BLE_GYRO_SAMPLES_PER_PACKET>
  s_ch_gyro_filt(BLE_TELEM_MASK_GYRO_FILT);
static BleTelemetryChannel<BleAccelPacket, BLE_ACCEL_SAMPLES_PER_PACKET>
  s_ch_accel_raw(BLE_TELEM_MASK_ACCEL_RAW);
static BleTelemetryChannel<BleAccelPacket, BLE_ACCEL_SAMPLES_PER_PACKET>
  s_ch_accel_filt(BLE_TELEM_MASK_ACCEL_FILT);
static BleTelemetryChannel<BleMagPacket, BLE_MAG_SAMPLES_PER_PACKET>
  s_ch_mag_raw(BLE_TELEM_MASK_MAG_RAW);
static BleTelemetryChannel<BleMagPacket, BLE_MAG_SAMPLES_PER_PACKET>
  s_ch_mag_filt(BLE_TELEM_MASK_MAG_FILT);
static BleTelemetryChannel<BleBaroPacket, BLE_BARO_SAMPLES_PER_PACKET>
  s_ch_baro_raw(BLE_TELEM_MASK_BARO_RAW);
static BleTelemetryChannel<BleBaroPacket, BLE_BARO_SAMPLES_PER_PACKET>
  s_ch_baro_filt(BLE_TELEM_MASK_BARO_FILT);
static BleTelemetryChannel<BlePowerPacket, BLE_POWER_SAMPLES_PER_PACKET>
  s_ch_power(BLE_TELEM_MASK_POWER);
static BleTelemetryChannel<BleRcPacket, BLE_RC_SAMPLES_PER_PACKET>
  s_ch_rc(BLE_TELEM_MASK_RC_CHANNELS);
static BleTelemetryChannel<BleAttQuatPacket, BLE_ATT_QUAT_SAMPLES_PER_PACKET>
  s_ch_att_quat(BLE_TELEM_MASK_ATT_QUAT);
static BleTelemetryChannel<BleAttEulerPacket, BLE_ATT_EULER_SAMPLES_PER_PACKET>
  s_ch_att_euler(BLE_TELEM_MASK_ATT_EULER);
static BleTelemetryChannel<BleAltPacket, BLE_ALT_SAMPLES_PER_PACKET>
  s_ch_alt(BLE_TELEM_MASK_ALT);

// =============================================================================
// UNIFIED BLE TELEMETRY DISPATCHER TASK (100 HZ EVALUATION CADENCE)
// =============================================================================
void ble_telem_task(void* arg){
  const uint32_t task_id = TASK_BLE_TELEM_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_BLE_TELEM_FREQ_HZ); // 10 ms (100 Hz)
  const TickType_t timeout_ticks = pdMS_TO_TICKS(1000 / BLE_TELEM_MIN_PACKET_FREQ_HZ); // 100 ms (10 Hz floor)

  TickType_t last_wake_time = xTaskGetTickCount();

  while(true){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2))
      last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(!is_ble_connected()) continue;

    SYSVIEW_START(task_id);

    // Process active telemetry streams with subscription backpressure
    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_GYRO_RAW) != 0)
      s_ch_gyro_raw.process(gyro_raw, raw_accel_gyro_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_GYRO_FILT) != 0)
      s_ch_gyro_filt.process(gyro, filtered_accel_gyro_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ACCEL_RAW) != 0)
      s_ch_accel_raw.process(accel_raw, raw_accel_gyro_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ACCEL_FILT) != 0)
      s_ch_accel_filt.process(accel, filtered_accel_gyro_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_MAG_RAW) != 0)
      s_ch_mag_raw.process(mag_raw, raw_mag_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_MAG_FILT) != 0)
      s_ch_mag_filt.process(mag, filtered_mag_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_BARO_RAW) != 0)
      s_ch_baro_raw.process(pressure, pressure_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_BARO_FILT) != 0)
      s_ch_baro_filt.process(pressure, pressure_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_POWER) != 0)
      s_ch_power.process(power_data, power_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_RC_CHANNELS) != 0)
      s_ch_rc.process(rc_data, rc_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ATT_QUAT) != 0)
      s_ch_att_quat.process(att_quat, att_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ATT_EULER) != 0)
      s_ch_att_euler.process(att_euler, att_timestamp, now, timeout_ticks);

    if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ALT) != 0)
      s_ch_alt.process(alt_state, alt_timestamp, now, timeout_ticks);

    SYSVIEW_END(task_id);
  }
}
