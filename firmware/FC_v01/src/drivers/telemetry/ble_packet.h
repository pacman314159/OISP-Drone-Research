#pragma once
#include <cstdint>
#include "core/math/matrix.h"

#pragma pack(push, 1)

// Generic 3-Axis Vector Telemetry Packet (16 Bytes Little-Endian)
template<typename T = float>
struct BleVec3Packet {
  uint32_t timestamp; // us or ms (per BLE_TELEM_TIMESTAMP_UNIT in config.h)
  Vec3<T>  data;      // rad/s (gyro), m/s^2 (accel), Gauss (mag), or deg (euler)

  BleVec3Packet() : timestamp(0), data(0, 0, 0){}
  BleVec3Packet(uint32_t ts, const Vec3<T>& val) : timestamp(ts), data(val){}
  BleVec3Packet(uint32_t ts, T x, T y, T z) : timestamp(ts), data(x, y, z){}
};

// Strongly-Typed Aliases for FCDatMon 16-Byte Stream Compatibility
using BleGyroPacket     = BleVec3Packet<float>;
using BleAccelPacket    = BleVec3Packet<float>;
using BleMagPacket      = BleVec3Packet<float>;
using BleAttEulerPacket = BleVec3Packet<float>;

// Barometer Raw & Filtered Telemetry Packet (8 Bytes Little-Endian)
struct BleBaroPacket {
  uint32_t timestamp; // us or ms
  float    pressure;  // Pa

  BleBaroPacket() : timestamp(0), pressure(0.0f){}
  BleBaroPacket(uint32_t ts, float p) : timestamp(ts), pressure(p){}
};

// Attitude Quaternion Telemetry Packet (20 Bytes Little-Endian)
struct BleAttQuatPacket {
  uint32_t timestamp; // us or ms
  float    q[4];      // w, x, y, z unit quaternion

  BleAttQuatPacket() : timestamp(0), q{1.0f, 0.0f, 0.0f, 0.0f}{}
  BleAttQuatPacket(uint32_t ts, float w, float x, float y, float z) : timestamp(ts), q{w, x, y, z}{}
};

// Power & Battery Telemetry Packet (20 Bytes Little-Endian, Millis Scale)
struct BlePowerPacket {
  uint32_t timestamp;  // us or ms
  float    voltage_mv; // mV
  float    current_ma; // mA
  float    power_mw;   // mW
  float    energy_mah; // mAh

  BlePowerPacket() : timestamp(0), voltage_mv(0.0f), current_ma(0.0f), power_mw(0.0f), energy_mah(0.0f){}
  template<typename TMeas>
  BlePowerPacket(uint32_t ts, const TMeas& m)
    : timestamp(ts), voltage_mv(m.voltage_mv), current_ma(m.current_ma), power_mw(m.power_mw), energy_mah(m.energy_mah){}
  BlePowerPacket(uint32_t ts, float v_mv, float i_ma, float p_mw, float e_mah)
    : timestamp(ts), voltage_mv(v_mv), current_ma(i_ma), power_mw(p_mw), energy_mah(e_mah){}
};

// RC Receiver Channels Telemetry Packet (28 Bytes Little-Endian, 6 Channels)
struct BleRcPacket {
  uint32_t timestamp; // us or ms
  float    ch[6];     // Channels 1..6 in microseconds (1000.0f - 2000.0f)

  BleRcPacket() : timestamp(0), ch{1500.0f, 1500.0f, 1000.0f, 1500.0f, 1000.0f, 1000.0f}{}
  template<typename TRc>
  BleRcPacket(uint32_t ts, const TRc& rc) : timestamp(ts){
    for(size_t i = 0; i < 6; ++i)
      ch[i] = (i < rc.channel_count) ? static_cast<float>(rc.channels[i]) : 1500.0f;
  }
  BleRcPacket(uint32_t ts, const float* in_ch) : timestamp(ts){
    for(size_t i = 0; i < 6; ++i)
      ch[i] = in_ch[i];
  }
};

#pragma pack(pop)

static_assert(sizeof(BleGyroPacket) == 16, "BleGyroPacket must be exactly 16 bytes for FCDatMon stride S=16");
static_assert(sizeof(BleAccelPacket) == 16, "BleAccelPacket must be exactly 16 bytes for FCDatMon stride S=16");
static_assert(sizeof(BleMagPacket) == 16, "BleMagPacket must be exactly 16 bytes for FCDatMon stride S=16");
static_assert(sizeof(BleAttEulerPacket) == 16, "BleAttEulerPacket must be exactly 16 bytes for FCDatMon stride S=16");
static_assert(sizeof(BleBaroPacket) == 8, "BleBaroPacket must be exactly 8 bytes for FCDatMon stride S=8");
static_assert(sizeof(BleAttQuatPacket) == 20, "BleAttQuatPacket must be exactly 20 bytes for FCDatMon stride S=20");
static_assert(sizeof(BlePowerPacket) == 20, "BlePowerPacket must be exactly 20 bytes for FCDatMon stride S=20");
static_assert(sizeof(BleRcPacket) == 28, "BleRcPacket must be exactly 28 bytes for FCDatMon stride S=28");
