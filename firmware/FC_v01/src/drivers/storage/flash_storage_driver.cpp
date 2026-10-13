#include "drivers/storage/flash_storage_driver.h"
#include <cstring>

FlashStorageDriver::FlashStorageDriver(HAL_Flash& flash_hal)
  : _hal(flash_hal), _initialized(false){
}

bool FlashStorageDriver::init(const char* partition_name){
  _initialized = _hal.init(partition_name);
  return _initialized;
}

uint32_t FlashStorageDriver::calculate_crc32(const uint8_t* data, size_t length){
  if(data == nullptr || length == 0) return 0;

  uint32_t crc = 0xFFFFFFFF;
  for(size_t i = 0; i < length; ++i){
    crc ^= data[i];
    for(uint8_t j = 0; j < 8; ++j){
      if(crc & 1)
        crc = (crc >> 1) ^ 0xEDB88320;
      else
        crc >>= 1;
    }
  }
  return ~crc;
}

bool FlashStorageDriver::read_record(const char* ns_name, const char* key, void* out_payload, size_t payload_len){
  if(!_initialized || ns_name == nullptr || key == nullptr || out_payload == nullptr || payload_len == 0) return false;
  if(payload_len > FLASH_MAX_RECORD_LEN) return false;

  uint8_t buffer[FLASH_MAX_RECORD_LEN + sizeof(FlashRecordHeader)];
  size_t bytes_read = 0;
  bool success = _hal.read_blob(ns_name, key, buffer, sizeof(buffer), &bytes_read);
  if(!success || bytes_read < (sizeof(FlashRecordHeader) + payload_len)) return false;

  FlashRecordHeader header;
  memcpy(&header, buffer, sizeof(FlashRecordHeader));

  if(header.magic != FLASH_RECORD_MAGIC) return false;
  if(header.schema_ver != FLASH_SCHEMA_VERSION) return false;
  if(header.payload_len != payload_len) return false;

  const uint8_t* payload_ptr = buffer + sizeof(FlashRecordHeader);
  uint32_t computed_crc = calculate_crc32(payload_ptr, payload_len);
  if(computed_crc != header.crc32) return false;

  memcpy(out_payload, payload_ptr, payload_len);
  return true;
}

bool FlashStorageDriver::write_record(const char* ns_name, const char* key, const void* payload, size_t payload_len){
  if(!_initialized || ns_name == nullptr || key == nullptr || payload == nullptr || payload_len == 0) return false;
  if(payload_len > FLASH_MAX_RECORD_LEN) return false;

  FlashRecordHeader header;
  header.magic       = FLASH_RECORD_MAGIC;
  header.schema_ver  = FLASH_SCHEMA_VERSION;
  header.payload_len = static_cast<uint16_t>(payload_len);
  header.crc32       = calculate_crc32(static_cast<const uint8_t*>(payload), payload_len);

  uint8_t buffer[FLASH_MAX_RECORD_LEN + sizeof(FlashRecordHeader)];
  memcpy(buffer, &header, sizeof(FlashRecordHeader));
  memcpy(buffer + sizeof(FlashRecordHeader), payload, payload_len);

  return _hal.write_blob(ns_name, key, buffer, sizeof(FlashRecordHeader) + payload_len);
}

bool FlashStorageDriver::erase_record(const char* ns_name, const char* key){
  if(!_initialized || ns_name == nullptr || key == nullptr) return false;
  return _hal.erase_key(ns_name, key);
}

bool FlashStorageDriver::erase_namespace(const char* ns_name){
  if(!_initialized || ns_name == nullptr) return false;
  return _hal.erase_namespace(ns_name);
}
