#pragma once
#include <cstdint>
#include <cstddef>

class HAL_Flash {
public:
  virtual ~HAL_Flash() = default;
  virtual bool init(const char* partition_name = "nvs") = 0;
  virtual bool read_blob(const char* ns_name, const char* key, void* out_data, size_t max_len, size_t* out_len = nullptr) = 0;
  virtual bool write_blob(const char* ns_name, const char* key, const void* data, size_t len) = 0;
  virtual bool erase_key(const char* ns_name, const char* key) = 0;
  virtual bool erase_namespace(const char* ns_name) = 0;
};

// Global Target Flash Accessor (Pure HAL interface, decoupled per Note 0002)
HAL_Flash& get_flash_storage();
