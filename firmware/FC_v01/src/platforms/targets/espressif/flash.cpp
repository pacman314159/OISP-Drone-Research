#include "platforms/targets/espressif/flash.h"
#include <cstring>
#include <cstdio>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"

ESP32_Flash::ESP32_Flash(){
  strncpy(_partition_name, "nvs", sizeof(_partition_name) - 1);
  _partition_name[sizeof(_partition_name) - 1] = '\0';
}

bool ESP32_Flash::init(const char* partition_name){
  if(partition_name != nullptr && strlen(partition_name) > 0){
    strncpy(_partition_name, partition_name, sizeof(_partition_name) - 1);
    _partition_name[sizeof(_partition_name) - 1] = '\0';
  }

  esp_err_t err = nvs_flash_init_partition(_partition_name);
  if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
    nvs_flash_erase_partition(_partition_name);
    err = nvs_flash_init_partition(_partition_name);
  }

  _initialized = (err == ESP_OK);
  return _initialized;
}

bool ESP32_Flash::read_blob(const char* ns_name, const char* key, void* out_data, size_t max_len, size_t* out_len){
  if(!_initialized || ns_name == nullptr || key == nullptr || out_data == nullptr || max_len == 0) return false;

  nvs_handle_t handle = 0;
  esp_err_t err = nvs_open_from_partition(_partition_name, ns_name, NVS_READONLY, &handle);
  if(err != ESP_OK) return false;

  size_t required_size = 0;
  err = nvs_get_blob(handle, key, nullptr, &required_size);
  if(err != ESP_OK || required_size > max_len){
    nvs_close(handle);
    return false;
  }

  err = nvs_get_blob(handle, key, out_data, &required_size);
  nvs_close(handle);

  if(err != ESP_OK) return false;
  if(out_len != nullptr) *out_len = required_size;
  return true;
}

bool ESP32_Flash::write_blob(const char* ns_name, const char* key, const void* data, size_t len){
  if(!_initialized || ns_name == nullptr || key == nullptr || data == nullptr || len == 0) return false;

  nvs_handle_t handle = 0;
  esp_err_t err = nvs_open_from_partition(_partition_name, ns_name, NVS_READWRITE, &handle);
  if(err != ESP_OK) return false;

  err = nvs_set_blob(handle, key, data, len);
  if(err == ESP_OK) err = nvs_commit(handle);
  nvs_close(handle);

  return (err == ESP_OK);
}

bool ESP32_Flash::erase_key(const char* ns_name, const char* key){
  if(!_initialized || ns_name == nullptr || key == nullptr) return false;

  nvs_handle_t handle = 0;
  esp_err_t err = nvs_open_from_partition(_partition_name, ns_name, NVS_READWRITE, &handle);
  if(err != ESP_OK) return false;

  err = nvs_erase_key(handle, key);
  if(err == ESP_OK) err = nvs_commit(handle);
  nvs_close(handle);

  return (err == ESP_OK);
}

bool ESP32_Flash::erase_namespace(const char* ns_name){
  if(!_initialized || ns_name == nullptr) return false;

  nvs_handle_t handle = 0;
  esp_err_t err = nvs_open_from_partition(_partition_name, ns_name, NVS_READWRITE, &handle);
  if(err != ESP_OK) return false;

  err = nvs_erase_all(handle);
  if(err == ESP_OK) err = nvs_commit(handle);
  nvs_close(handle);

  return (err == ESP_OK);
}

HAL_Flash& get_flash_storage(){
  static ESP32_Flash s_instance;
  return s_instance;
}
