#pragma once
#include <cstdint>
#include <cstddef>

// =============================================================================
// HARDWARE ABSTRACTION — OTA (Layer 1/HAL)
// =============================================================================
// C7: firmware-update primitives are hardware interaction and must not be
// called directly from Layer 4. This interface exposes only primitive
// parameters (sizes, buffers, status), so the OTA task above stays testable
// and target-neutral.

enum class OtaStatus : uint8_t {
  OK = 0,
  NO_PARTITION,        // no valid update partition
  FIRMWARE_TOO_LARGE,  // image does not fit the update partition
  FLASH_ERROR,         // begin/write/set-boot failed
  VERIFY_FAILED        // image validation failed
};

class HAL_OTA {
public:
  virtual ~HAL_OTA() = default;

  // Selects the next update partition, validates capacity and opens a handle.
  virtual OtaStatus begin(uint32_t fw_size_bytes) = 0;
  virtual bool      write(const uint8_t* data, uint32_t len) = 0;
  // Finalises the image and marks the update partition as next boot target.
  virtual OtaStatus end_and_set_boot() = 0;
  virtual void      abort() = 0;
  virtual void      restart() = 0;
};

// Global Target OTA Accessor (Pure HAL interface, decoupled per Note 0002)
HAL_OTA& get_ota_hal();
