#pragma once
#include <cstdint>

// OTA Protocol Commands & Status Response Codes
enum OtaCommandCode : uint8_t {
  OTA_CMD_START  = 0x01,
  OTA_CMD_END    = 0x02,
  OTA_CMD_ABORT  = 0x03,
  OTA_CMD_REBOOT = 0x04
};

enum OtaResponseCode : uint8_t {
  OTA_RESP_OK         = 0x00,
  OTA_RESP_READY_DATA = 0x01,
  OTA_RESP_PROGRESS   = 0x02,
  OTA_RESP_SUCCESS    = 0x03,
  OTA_RESP_ERR_SIZE   = 0x81,
  OTA_RESP_ERR_FLASH  = 0x82,
  OTA_RESP_ERR_VERIFY = 0x83,
  OTA_RESP_ERR_ABORT  = 0x84
};

#pragma pack(push, 1)
struct OtaStartPayload {
  uint8_t  cmd;             // OTA_CMD_START (0x01)
  uint32_t fw_size_bytes;   // Firmware image size in bytes
  uint32_t crc32_checksum;  // Optional CRC32 checksum (0 if unused)
};

struct OtaProgressPayload {
  uint8_t  code;            // OTA_RESP_PROGRESS (0x02)
  uint32_t bytes_written;   // Bytes flashed so far
  uint8_t  percent;         // 0 - 100%
};

struct OtaEndPayload {
  uint8_t  cmd;             // OTA_CMD_END (0x02)
  uint32_t fw_size_bytes;   // Total expected bytes
};
#pragma pack(pop)

constexpr uint16_t OTA_CHUNK_EOF_MARKER   = 0x0000;
constexpr uint16_t OTA_CHUNK_ABORT_MARKER = 0xFFFE;

struct OtaDataChunk {
  uint16_t length;
  uint8_t  data[512];
};

extern OtaStartPayload ota_start_payload;
extern volatile bool   ota_abort_requested;
