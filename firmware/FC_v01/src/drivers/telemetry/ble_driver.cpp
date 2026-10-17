#include <Arduino.h>
#include <NimBLEDevice.h>
#include <esp_log.h>
#include "config.h"
#include "middleware/ipc.h"
#include "drivers/telemetry/ble_driver.h"
#include "drivers/telemetry/ble_ota_packet.h"

static NimBLECharacteristic* p_gyro_raw_char   = nullptr;
static NimBLECharacteristic* p_gyro_filt_char  = nullptr;
static NimBLECharacteristic* p_accel_raw_char  = nullptr;
static NimBLECharacteristic* p_accel_filt_char = nullptr;
static NimBLECharacteristic* p_mag_raw_char    = nullptr;
static NimBLECharacteristic* p_mag_filt_char   = nullptr;
static NimBLECharacteristic* p_baro_raw_char   = nullptr;
static NimBLECharacteristic* p_baro_filt_char  = nullptr;
static NimBLECharacteristic* p_att_quat_char   = nullptr;
static NimBLECharacteristic* p_att_euler_char  = nullptr;
static NimBLECharacteristic* p_power_char      = nullptr;
static NimBLECharacteristic* p_rc_channels_char = nullptr;
static NimBLECharacteristic* p_ota_control_char = nullptr;
static NimBLECharacteristic* p_ota_data_char    = nullptr;
static bool ble_connected                       = false;
static bool ble_ota_active                      = false;

bool is_ble_connected(){
  return ble_connected;
}

void set_ble_connected(bool connected){
  ble_connected = connected;
}

bool is_ble_ota_active(){
  return ble_ota_active;
}

void set_ble_ota_active(bool active){
  ble_ota_active = active;
}

class OtaControlCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic) override{
    std::string val = pCharacteristic->getValue();
    if(val.empty()) return;

    const uint8_t* data = reinterpret_cast<const uint8_t*>(val.data());
    uint8_t cmd = data[0];

    if(cmd == OTA_CMD_START){
      if(val.length() >= sizeof(OtaStartPayload))
        memcpy(&ota_start_payload, data, sizeof(OtaStartPayload));
      else{
        ota_start_payload.cmd = OTA_CMD_START;
        ota_start_payload.fw_size_bytes = 0; // bytes
        ota_start_payload.crc32_checksum = 0;
      }
      ota_abort_requested = false;
      if(app_task_handles[TASK_BLE_OTA_ID] != nullptr){
        xTaskNotifyGive(app_task_handles[TASK_BLE_OTA_ID]);
      }else{
        Serial.println("[BLE_OTA] OTA trigger rejected: Boot window expired.");
        uint8_t err_resp = OTA_RESP_ERR_ABORT;
        send_ble_ota_response(&err_resp, sizeof(err_resp));
      }
    }else if(cmd == OTA_CMD_END){
      if(ota_data_queue != nullptr){
        OtaDataChunk eof_chunk = {};
        eof_chunk.length = OTA_CHUNK_EOF_MARKER;
        if(val.length() >= sizeof(OtaEndPayload)){
          OtaEndPayload end_payload;
          memcpy(&end_payload, data, sizeof(OtaEndPayload));
          memcpy(eof_chunk.data, &end_payload.fw_size_bytes, sizeof(uint32_t));
        }
        xQueueSend(ota_data_queue, &eof_chunk, pdMS_TO_TICKS(100));
      }
    }else if(cmd == OTA_CMD_ABORT){
      ota_abort_requested = true;
      if(ota_data_queue != nullptr){
        OtaDataChunk abort_chunk = {};
        abort_chunk.length = OTA_CHUNK_ABORT_MARKER;
        xQueueSend(ota_data_queue, &abort_chunk, 0);
      }
      if(app_task_handles[TASK_BLE_OTA_ID] != nullptr)
        xTaskNotifyGive(app_task_handles[TASK_BLE_OTA_ID]);
    }
  }
};

static OtaDataChunk s_incoming_ota_chunk;

class OtaDataCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic) override{
    if(!ble_ota_active || ota_data_queue == nullptr) return;

    std::string val = pCharacteristic->getValue();
    if(val.empty()) return;

    s_incoming_ota_chunk.length = static_cast<uint16_t>(val.length() > sizeof(s_incoming_ota_chunk.data) ? sizeof(s_incoming_ota_chunk.data) : val.length());
    memcpy(s_incoming_ota_chunk.data, val.data(), s_incoming_ota_chunk.length);

    if(xQueueSend(ota_data_queue, &s_incoming_ota_chunk, pdMS_TO_TICKS(100)) != pdTRUE){
      Serial.println("[BLE_OTA] Error: ota_data_queue overflow! Aborting session.");
      ota_abort_requested = true;
      if(app_task_handles[TASK_BLE_OTA_ID] != nullptr)
        xTaskNotifyGive(app_task_handles[TASK_BLE_OTA_ID]);
    }
  }
};

static OtaControlCallbacks ota_control_callbacks;
static OtaDataCallbacks ota_data_callbacks;

bool ble_driver_init(){
  esp_log_level_set("NimBLE", ESP_LOG_WARN);

  NimBLEDevice::init(BLE_DEVICE_NAME);
  NimBLEDevice::setMTU(BLE_MTU_SIZE);

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&ble_server_callbacks);

  // Initialize Telemetry GATT Service & Characteristics conditionally based on SSoT Mask
  NimBLEService* pTelemetryService = pServer->createService(BLE_SERVICE_TELEM_UUID);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_GYRO_RAW) != 0)
    p_gyro_raw_char = pTelemetryService->createCharacteristic(BLE_CHAR_GYRO_RAW_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_GYRO_FILT) != 0)
    p_gyro_filt_char = pTelemetryService->createCharacteristic(BLE_CHAR_GYRO_FILT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ACCEL_RAW) != 0)
    p_accel_raw_char = pTelemetryService->createCharacteristic(BLE_CHAR_ACCEL_RAW_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ACCEL_FILT) != 0)
    p_accel_filt_char = pTelemetryService->createCharacteristic(BLE_CHAR_ACCEL_FILT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_MAG_RAW) != 0)
    p_mag_raw_char = pTelemetryService->createCharacteristic(BLE_CHAR_MAG_RAW_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_MAG_FILT) != 0)
    p_mag_filt_char = pTelemetryService->createCharacteristic(BLE_CHAR_MAG_FILT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_BARO_RAW) != 0)
    p_baro_raw_char = pTelemetryService->createCharacteristic(BLE_CHAR_BARO_RAW_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_BARO_FILT) != 0)
    p_baro_filt_char = pTelemetryService->createCharacteristic(BLE_CHAR_BARO_FILT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ATT_QUAT) != 0)
    p_att_quat_char = pTelemetryService->createCharacteristic(BLE_CHAR_ATT_QUAT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_ATT_EULER) != 0)
    p_att_euler_char = pTelemetryService->createCharacteristic(BLE_CHAR_ATT_EULER_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_POWER) != 0)
    p_power_char = pTelemetryService->createCharacteristic(BLE_CHAR_POWER_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  if constexpr((ACTIVE_BLE_TELEM_MASK & BLE_TELEM_MASK_RC_CHANNELS) != 0)
    p_rc_channels_char = pTelemetryService->createCharacteristic(BLE_CHAR_RC_CHANNELS_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  pTelemetryService->start();

  // Initialize OTA GATT Service & Characteristics
  NimBLEService* pOtaService = pServer->createService(BLE_SERVICE_OTA_UUID);
  p_ota_control_char = pOtaService->createCharacteristic(
    BLE_CHAR_OTA_CONTROL_UUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE
  );
  p_ota_control_char->setCallbacks(&ota_control_callbacks);

  p_ota_data_char = pOtaService->createCharacteristic(
    BLE_CHAR_OTA_DATA_UUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  p_ota_data_char->setCallbacks(&ota_data_callbacks);

  pOtaService->start();

  // Start BLE Advertising
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_TELEM_UUID);
  pAdvertising->addServiceUUID(BLE_SERVICE_OTA_UUID);
  pAdvertising->setName(BLE_DEVICE_NAME);
  pAdvertising->start();

  Serial.println("NimBLE Driver Initialized & Advertising Started.");
  return true;
}

static NimBLECharacteristic* get_char_by_mask(uint16_t mask_bit){
  switch(mask_bit){
    case BLE_TELEM_MASK_GYRO_RAW:    return p_gyro_raw_char;
    case BLE_TELEM_MASK_GYRO_FILT:   return p_gyro_filt_char;
    case BLE_TELEM_MASK_ACCEL_RAW:   return p_accel_raw_char;
    case BLE_TELEM_MASK_ACCEL_FILT:  return p_accel_filt_char;
    case BLE_TELEM_MASK_MAG_RAW:     return p_mag_raw_char;
    case BLE_TELEM_MASK_MAG_FILT:    return p_mag_filt_char;
    case BLE_TELEM_MASK_BARO_RAW:    return p_baro_raw_char;
    case BLE_TELEM_MASK_BARO_FILT:   return p_baro_filt_char;
    case BLE_TELEM_MASK_ATT_QUAT:    return p_att_quat_char;
    case BLE_TELEM_MASK_ATT_EULER:   return p_att_euler_char;
    case BLE_TELEM_MASK_POWER:       return p_power_char;
    case BLE_TELEM_MASK_RC_CHANNELS: return p_rc_channels_char;
    default:                         return nullptr;
  }
}

static bool send_char_notification(NimBLECharacteristic* p_char, const uint8_t* data, size_t len){
  if(p_char == nullptr || !is_ble_connected()) return false;
  if(p_char->getSubscribedCount() == 0) return false;

  p_char->setValue(data, len);
  p_char->notify();
  return true;
}

bool is_ble_stream_subscribed(uint16_t mask_bit){
  NimBLECharacteristic* p_char = get_char_by_mask(mask_bit);
  if(p_char == nullptr || !is_ble_connected()) return false;
  return (p_char->getSubscribedCount() > 0);
}

bool send_ble_stream_notification(uint16_t mask_bit, const uint8_t* data, size_t len){
  NimBLECharacteristic* p_char = get_char_by_mask(mask_bit);
  return send_char_notification(p_char, data, len);
}

bool send_ble_ota_response(const uint8_t* data, size_t len){
  if(p_ota_control_char == nullptr || !is_ble_connected()) return false;
  p_ota_control_char->setValue(data, len);
  p_ota_control_char->notify();
  return true;
}

