#pragma once
#include <cstdint>
#include "src/core/math/matrix.h"

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

#pragma pack(pop)

static_assert(sizeof(BleGyroPacket) == 16, "BleGyroPacket must be exactly 16 bytes for FCDatMon stride S=16");
static_assert(sizeof(BleAccelPacket) == 16, "BleAccelPacket must be exactly 16 bytes for FCDatMon stride S=16");
static_assert(sizeof(BleMagPacket) == 16, "BleMagPacket must be exactly 16 bytes for FCDatMon stride S=16");
static_assert(sizeof(BleAttEulerPacket) == 16, "BleAttEulerPacket must be exactly 16 bytes for FCDatMon stride S=16");
static_assert(sizeof(BleBaroPacket) == 8, "BleBaroPacket must be exactly 8 bytes for FCDatMon stride S=8");
static_assert(sizeof(BleAttQuatPacket) == 20, "BleAttQuatPacket must be exactly 20 bytes for FCDatMon stride S=20");
