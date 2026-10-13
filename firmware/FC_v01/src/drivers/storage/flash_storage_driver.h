#pragma once
#include <cstdint>
#include <cstddef>
#include "platforms/hal/hal_flash.h"

constexpr uint32_t FLASH_RECORD_MAGIC   = 0xCA11B001;
constexpr uint16_t FLASH_SCHEMA_VERSION = 1;
constexpr size_t   FLASH_MAX_RECORD_LEN = 128;

struct __attribute__((packed)) FlashRecordHeader {
  uint32_t magic;        // FLASH_RECORD_MAGIC (0xCA11B001)
  uint16_t schema_ver;   // Schema revision counter
  uint16_t payload_len;  // Length of raw payload bytes following header
  uint32_t crc32;        // IEEE 802.3 CRC32 checksum over payload
};

class FlashStorageDriver {
public:
  explicit FlashStorageDriver(HAL_Flash& flash_hal);
  ~FlashStorageDriver() = default;

  bool init(const char* partition_name = "nvs");
  bool read_record(const char* ns_name, const char* key, void* out_payload, size_t payload_len);
  bool write_record(const char* ns_name, const char* key, const void* payload, size_t payload_len);
  bool erase_record(const char* ns_name, const char* key);
  bool erase_namespace(const char* ns_name);

  static uint32_t calculate_crc32(const uint8_t* data, size_t length);

private:
  HAL_Flash& _hal;
  bool _initialized{false};
};
