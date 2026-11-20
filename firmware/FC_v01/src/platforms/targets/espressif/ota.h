#pragma once
#include "platforms/hal/hal_ota.h"

class ESP32_OTA : public HAL_OTA {
public:
  ~ESP32_OTA() override = default;

  OtaStatus begin(uint32_t fw_size_bytes) override;
  bool      write(const uint8_t* data, uint32_t len) override;
  OtaStatus end_and_set_boot() override;
  void      abort() override;
  void      restart() override;

private:
  const void* _update_partition{nullptr};   // esp_partition_t*, opaque in the header
  uint32_t    _handle{0};                   // esp_ota_handle_t
  bool        _begun{false};
};
