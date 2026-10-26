#include <Arduino.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include "ble_ota_tasks.h"
#include "config.h"
#include "middleware/ipc.h"
#include "drivers/telemetry/ble_driver.h"
#include "drivers/telemetry/ble_ota_packet.h"
#include "drivers/led/ws2812.h"
#include "app/app.h"

void ble_ota_task(void* arg){
  (void)arg;
  const uint32_t task_id = TASK_BLE_OTA_ID;
  (void)task_id;
  Serial.printf("[BLE_OTA] Task started. Listening for OTA trigger (%lu ms window)...\n", static_cast<unsigned long>(OTA_BOOT_WINDOW_TIMEOUT_MS));

  uint32_t notified = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(OTA_BOOT_WINDOW_TIMEOUT_MS));

  if(notified == 0){
    Serial.printf("[BLE_OTA] %lu ms boot window expired without OTA trigger. Terminating OTA task.\n", static_cast<unsigned long>(OTA_BOOT_WINDOW_TIMEOUT_MS));
    set_ble_ota_active(false);
    app_task_handles[TASK_BLE_OTA_ID] = nullptr;
    if(ota_data_queue != nullptr){
      vQueueDelete(ota_data_queue);
      ota_data_queue = nullptr;
    }
    vTaskDelete(NULL);
    return;
  }

  Serial.println("[BLE_OTA] OTA trigger received! Entering OTA Update Mode...");
  set_ble_ota_active(true);

  app_suspend_all_tasks();
  Serial.println("[BLE_OTA] All flight tasks suspended.");

  const esp_partition_t* running = esp_ota_get_running_partition();
  const esp_partition_t* update_partition = esp_ota_get_next_update_partition(NULL);

  if(update_partition == nullptr){
    Serial.println("[BLE_OTA] Error: No valid OTA update partition found!");
    uint8_t err_resp = OTA_RESP_ERR_FLASH;
    send_ble_ota_response(&err_resp, sizeof(err_resp));
    set_ble_ota_active(false);
    app_task_handles[TASK_BLE_OTA_ID] = nullptr;
    vTaskDelete(NULL);
    return;
  }

  if(running != nullptr)
    Serial.printf("[BLE_OTA] Running partition: %s, Target partition: %s (size: 0x%lx)\n",
                  running->label, update_partition->label, static_cast<unsigned long>(update_partition->size));

  uint32_t fw_size = ota_start_payload.fw_size_bytes; // bytes
  if(fw_size > update_partition->size){
    Serial.printf("[BLE_OTA] Error: Firmware size %lu exceeds partition capacity %lu!\n",
                  static_cast<unsigned long>(fw_size), static_cast<unsigned long>(update_partition->size));
    uint8_t err_resp = OTA_RESP_ERR_SIZE;
    send_ble_ota_response(&err_resp, sizeof(err_resp));
    set_ble_ota_active(false);
    app_task_handles[TASK_BLE_OTA_ID] = nullptr;
    vTaskDelete(NULL);
    return;
  }

  esp_ota_handle_t ota_handle = 0;
  esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
  if(err != ESP_OK){
    Serial.printf("[BLE_OTA] esp_ota_begin failed: 0x%x\n", err);
    uint8_t err_resp = OTA_RESP_ERR_FLASH;
    send_ble_ota_response(&err_resp, sizeof(err_resp));
    set_ble_ota_active(false);
    app_task_handles[TASK_BLE_OTA_ID] = nullptr;
    vTaskDelete(NULL);
    return;
  }

  Serial.println("[BLE_OTA] Partition initialized. Ready to receive binary chunks.");
  uint8_t ready_resp = OTA_RESP_READY_DATA;
  send_ble_ota_response(&ready_resp, sizeof(ready_resp));

  uint32_t total_written = 0; // bytes
  uint8_t last_percent = 0;
  bool ota_success = false;
  OtaDataChunk chunk;

  while(true){
    if(ota_abort_requested || !is_ble_connected()){
      Serial.println("[BLE_OTA] Transfer interrupted (abort requested or connection lost). Discarding image...");
      esp_ota_abort(ota_handle);
      uint8_t err_resp = OTA_RESP_ERR_ABORT;
      send_ble_ota_response(&err_resp, sizeof(err_resp));
      break;
    }

    if(xQueueReceive(ota_data_queue, &chunk, pdMS_TO_TICKS(3000)) != pdTRUE){
      Serial.println("[BLE_OTA] Data stream timed out or connection dropped! Discarding image...");
      esp_ota_abort(ota_handle);
      uint8_t err_resp = OTA_RESP_ERR_FLASH;
      send_ble_ota_response(&err_resp, sizeof(err_resp));
      break;
    }

    if(chunk.length == OTA_CHUNK_ABORT_MARKER){
      Serial.println("[BLE_OTA] Abort marker received. Discarding image...");
      esp_ota_abort(ota_handle);
      uint8_t err_resp = OTA_RESP_ERR_ABORT;
      send_ble_ota_response(&err_resp, sizeof(err_resp));
      break;
    }

    if(chunk.length == OTA_CHUNK_EOF_MARKER){
      uint32_t expected_total = fw_size;
      uint32_t embedded_size = 0;
      memcpy(&embedded_size, chunk.data, sizeof(uint32_t));
      if(embedded_size > 0)
        expected_total = embedded_size;

      if(total_written != expected_total){
        Serial.printf("[BLE_OTA] Size mismatch: written %lu, expected %lu bytes. Discarding image...\n",
                      static_cast<unsigned long>(total_written), static_cast<unsigned long>(expected_total));
        esp_ota_abort(ota_handle);
        uint8_t err_resp = OTA_RESP_ERR_VERIFY;
        send_ble_ota_response(&err_resp, sizeof(err_resp));
        break;
      }

      Serial.printf("[BLE_OTA] End of data received. Total bytes written: %lu. Validating image...\n",
                    static_cast<unsigned long>(total_written));
      err = esp_ota_end(ota_handle);
      if(err != ESP_OK){
        Serial.printf("[BLE_OTA] esp_ota_end validation failed: 0x%x\n", err);
        uint8_t err_resp = OTA_RESP_ERR_VERIFY;
        send_ble_ota_response(&err_resp, sizeof(err_resp));
        break;
      }

      err = esp_ota_set_boot_partition(update_partition);
      if(err != ESP_OK){
        Serial.printf("[BLE_OTA] esp_ota_set_boot_partition failed: 0x%x\n", err);
        uint8_t err_resp = OTA_RESP_ERR_FLASH;
        send_ble_ota_response(&err_resp, sizeof(err_resp));
        break;
      }

      Serial.println("[BLE_OTA] Firmware update SUCCESSFUL! New boot partition set.");
      uint8_t success_resp = OTA_RESP_SUCCESS;
      send_ble_ota_response(&success_resp, sizeof(success_resp));
      ota_success = true;
      break;
    }

    err = esp_ota_write(ota_handle, chunk.data, chunk.length);
    if(err != ESP_OK){
      Serial.printf("[BLE_OTA] esp_ota_write failed: 0x%x\n", err);
      esp_ota_abort(ota_handle);
      uint8_t err_resp = OTA_RESP_ERR_FLASH;
      send_ble_ota_response(&err_resp, sizeof(err_resp));
      break;
    }

    total_written += chunk.length;

    if(fw_size > 0){
      uint8_t percent = static_cast<uint8_t>((static_cast<uint64_t>(total_written) * 100) / fw_size);
      if(percent >= last_percent + 5 || percent == 100){
        last_percent = percent;
        OtaProgressPayload prog;
        prog.code = OTA_RESP_PROGRESS;
        prog.bytes_written = total_written;
        prog.percent = percent;
        send_ble_ota_response(reinterpret_cast<const uint8_t*>(&prog), sizeof(prog));
      }
    }
  }

  set_ble_ota_active(false);
  app_task_handles[TASK_BLE_OTA_ID] = nullptr;

  const uint32_t reboot_delay_ms = ota_success ? 500 : 1000;
  if(ota_success){
    Serial.println("[BLE_OTA] Rebooting system into new firmware in 500 ms...");
  }else{
    Serial.println("[BLE_OTA] OTA Failed or Aborted. Rebooting system in 1000 ms...");
  }
  vTaskDelay(pdMS_TO_TICKS(reboot_delay_ms));
  esp_restart();
}
