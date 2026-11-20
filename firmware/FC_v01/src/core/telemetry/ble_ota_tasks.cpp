#include <Arduino.h>
#include "ble_ota_tasks.h"
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/sysview_tracing.h"
#include "drivers/telemetry/ble_driver.h"
#include "drivers/telemetry/ble_ota_packet.h"
#include "drivers/led/ws2812.h"
#include "platforms/hal/hal_ota.h"

void ble_ota_task(void* arg){
  HAL_OTA& ota = *static_cast<HAL_OTA*>(arg);
  const uint32_t task_id = TASK_BLE_OTA_ID;

  Serial.println("[BLE_OTA] OTA trigger received! Entering OTA Update Mode...");
  set_ble_ota_active(true);

  const uint32_t fw_size = ota_start_payload.fw_size_bytes; // bytes

  // [C7] Partition selection, capacity check and esp_ota_begin live behind HAL_OTA.
  SYSVIEW_START(task_id);
  OtaStatus begin_status = ota.begin(fw_size);
  SYSVIEW_END(task_id);

  if(begin_status != OtaStatus::OK){
    uint8_t err_resp = (begin_status == OtaStatus::FIRMWARE_TOO_LARGE)
                       ? OTA_RESP_ERR_SIZE : OTA_RESP_ERR_FLASH;
    send_ble_ota_response(&err_resp, sizeof(err_resp));
    set_ble_ota_active(false);
    ipc_register_task(TASK_BLE_OTA_ID, nullptr);   // C4: registry through the Layer 3 API
    ipc_destroy_ota_queue();
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

  // Event-driven task: the marker covers ONE chunk of work, never the blocked
  // queue wait (up to 3 s of timeout), so the trace shows real flash/CPU cost.
  bool iter_open = false;
  auto close_iteration = [&iter_open](){
    if(iter_open){
      SYSVIEW_END(TASK_BLE_OTA_ID);
      iter_open = false;
    }
  };

  while(true){
    if(ota_abort_requested || !is_ble_connected()){
      Serial.println("[BLE_OTA] Transfer interrupted (abort requested or connection lost). Discarding image...");
      ota.abort();
      uint8_t err_resp = OTA_RESP_ERR_ABORT;
      send_ble_ota_response(&err_resp, sizeof(err_resp));
      break;
    }

    if(xQueueReceive(ota_data_queue, &chunk, pdMS_TO_TICKS(3000)) != pdTRUE){
      Serial.println("[BLE_OTA] Data stream timed out or connection dropped! Discarding image...");
      ota.abort();
      uint8_t err_resp = OTA_RESP_ERR_FLASH;
      send_ble_ota_response(&err_resp, sizeof(err_resp));
      break;
    }

    SYSVIEW_START(task_id);
    iter_open = true;

    if(chunk.length == OTA_CHUNK_ABORT_MARKER){
      Serial.println("[BLE_OTA] Abort marker received. Discarding image...");
      ota.abort();
      uint8_t err_resp = OTA_RESP_ERR_ABORT;
      send_ble_ota_response(&err_resp, sizeof(err_resp));
      close_iteration();
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
        ota.abort();
        uint8_t err_resp = OTA_RESP_ERR_VERIFY;
        send_ble_ota_response(&err_resp, sizeof(err_resp));
        close_iteration();
        break;
      }

      Serial.printf("[BLE_OTA] End of data received. Total bytes written: %lu. Validating image...\n",
                    static_cast<unsigned long>(total_written));

      OtaStatus end_status = ota.end_and_set_boot();
      if(end_status != OtaStatus::OK){
        uint8_t err_resp = (end_status == OtaStatus::VERIFY_FAILED)
                           ? OTA_RESP_ERR_VERIFY : OTA_RESP_ERR_FLASH;
        send_ble_ota_response(&err_resp, sizeof(err_resp));
        close_iteration();
        break;
      }

      Serial.println("[BLE_OTA] Firmware update SUCCESSFUL! New boot partition set.");
      uint8_t success_resp = OTA_RESP_SUCCESS;
      send_ble_ota_response(&success_resp, sizeof(success_resp));
      ota_success = true;
      close_iteration();
      break;
    }

    if(!ota.write(chunk.data, chunk.length)){
      ota.abort();
      uint8_t err_resp = OTA_RESP_ERR_FLASH;
      send_ble_ota_response(&err_resp, sizeof(err_resp));
      close_iteration();
      break;
    }

    total_written += chunk.length;

    // Yield briefly every ~4 KB flash sector boundary to prevent NimBLE stack & IDLE watchdog starvation
    if((total_written % 4096) < sizeof(chunk.data))
      vTaskDelay(pdMS_TO_TICKS(1));

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

    close_iteration();
  }

  set_ble_ota_active(false);
  ipc_register_task(TASK_BLE_OTA_ID, nullptr);   // C4

  const uint32_t reboot_delay_ms = ota_success ? 500 : 1000;
  if(ota_success){
    Serial.println("[BLE_OTA] Rebooting system into new firmware in 500 ms...");
  }else{
    Serial.println("[BLE_OTA] OTA Failed or Aborted. Rebooting system in 1000 ms...");
  }
  vTaskDelay(pdMS_TO_TICKS(reboot_delay_ms));
  ota.restart();
}

bool spawn_ble_ota_task(HAL_OTA& ota){
  if(ipc_get_task(TASK_BLE_OTA_ID) != nullptr) return true;   // C4
  if(!ipc_create_ota_queue(sizeof(OtaDataChunk))) return false;

  TaskHandle_t handle = nullptr;
  BaseType_t ret = xTaskCreatePinnedToCore(
    ble_ota_task,
    TASK_BLE_OTA_NAME,
    TASK_BLE_OTA_STACK_SIZE,
    &ota,
    TASK_BLE_OTA_PRIORITY,
    &handle,
    TASK_BLE_OTA_CORE
  );
  if(ret != pdPASS) return false;

  ipc_register_task(TASK_BLE_OTA_ID, handle);   // C4: no direct registry write
  return true;
}
