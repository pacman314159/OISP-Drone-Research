#pragma once
#include "platforms/hal/hal_flash.h"

class ESP32_Flash : public HAL_Flash {
public:
  ESP32_Flash();
  ~ESP32_Flash() override = default;

  bool init(const char* partition_name = "nvs") override;
  bool read_blob(const char* ns_name, const char* key, void* out_data, size_t max_len, size_t* out_len = nullptr) override;
  bool write_blob(const char* ns_name, const char* key, const void* data, size_t len) override;
  bool erase_key(const char* ns_name, const char* key) override;
  bool erase_namespace(const char* ns_name) override;

private:
  char _partition_name[16]{"nvs"};
  bool _initialized{false};
};
