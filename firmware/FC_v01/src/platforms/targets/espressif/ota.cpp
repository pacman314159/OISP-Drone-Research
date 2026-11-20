#include "platforms/targets/espressif/ota.h"
#include <Arduino.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

// C7: the ESP-IDF OTA primitives moved here verbatim from
// src/core/telemetry/ble_ota_tasks.cpp. Layer 4 now only sees HAL_OTA.

OtaStatus ESP32_OTA::begin(uint32_t fw_size_bytes){
  const esp_partition_t* running = esp_ota_get_running_partition();
  const esp_partition_t* update = esp_ota_get_next_update_partition(nullptr);
  _update_partition = update;

  if(update == nullptr){
    Serial.println("[HAL_OTA] Error: No valid OTA update partition found!");
    return OtaStatus::NO_PARTITION;
  }

  if(running != nullptr)
    Serial.printf("[HAL_OTA] Running partition: %s, Target partition: %s (size: 0x%lx)\n",
                  running->label, update->label,
                  static_cast<unsigned long>(update->size));

  if(fw_size_bytes > update->size){
    Serial.printf("[HAL_OTA] Error: Firmware size %lu exceeds partition capacity %lu!\n",
                  static_cast<unsigned long>(fw_size_bytes),
                  static_cast<unsigned long>(update->size));
    return OtaStatus::FIRMWARE_TOO_LARGE;
  }

  esp_ota_handle_t handle = 0;
  esp_err_t err = esp_ota_begin(update, OTA_WITH_SEQUENTIAL_WRITES, &handle);
  if(err != ESP_OK){
    Serial.printf("[HAL_OTA] esp_ota_begin failed: 0x%x\n", err);
    return OtaStatus::FLASH_ERROR;
  }

  _handle = handle;
  _begun = true;
  Serial.println("[HAL_OTA] Partition initialized. Ready to receive binary chunks.");
  return OtaStatus::OK;
}

bool ESP32_OTA::write(const uint8_t* data, uint32_t len){
  if(!_begun || data == nullptr) return false;

  esp_err_t err = esp_ota_write(static_cast<esp_ota_handle_t>(_handle), data, len);
  if(err != ESP_OK){
    Serial.printf("[HAL_OTA] esp_ota_write failed: 0x%x\n", err);
    return false;
  }
  return true;
}

OtaStatus ESP32_OTA::end_and_set_boot(){
  if(!_begun) return OtaStatus::FLASH_ERROR;

  esp_err_t err = esp_ota_end(static_cast<esp_ota_handle_t>(_handle));
  _begun = false;
  if(err != ESP_OK){
    Serial.printf("[HAL_OTA] esp_ota_end validation failed: 0x%x\n", err);
    return OtaStatus::VERIFY_FAILED;
  }

  err = esp_ota_set_boot_partition(static_cast<const esp_partition_t*>(_update_partition));
  if(err != ESP_OK){
    Serial.printf("[HAL_OTA] esp_ota_set_boot_partition failed: 0x%x\n", err);
    return OtaStatus::FLASH_ERROR;
  }
  return OtaStatus::OK;
}

void ESP32_OTA::abort(){
  if(!_begun) return;
  esp_ota_abort(static_cast<esp_ota_handle_t>(_handle));
  _begun = false;
}

void ESP32_OTA::restart(){
  esp_restart();
}

HAL_OTA& get_ota_hal(){
  static ESP32_OTA s_instance;
  return s_instance;
}
