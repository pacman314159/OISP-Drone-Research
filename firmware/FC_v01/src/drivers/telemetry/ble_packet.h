#pragma once
#include <cstdint>
#include "src/core/math/matrix.h"

#pragma pack(push, 1)
template<typename T = float>
struct BleVec3Packet {
  uint32_t timestamp;
  Vec3<T> data;

  BleVec3Packet() : timestamp(0), data(0, 0, 0){}
  BleVec3Packet(uint32_t ts, const Vec3<T>& val) : timestamp(ts), data(val){}
  BleVec3Packet(uint32_t ts, T x, T y, T z) : timestamp(ts), data(x, y, z){}
};
#pragma pack(pop)
