#pragma once
#include <cstdint>

enum class SafetyStatus : uint8_t {
  PENDING = 0,
  PASSED,
  FAILED
};
