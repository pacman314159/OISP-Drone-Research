#include <Arduino.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "middleware/ipc.h"
#include "drivers/telemetry/ble_driver.h"

static NimBLECharacteristic* p_gyro_char = nullptr;
static bool ble_connected = false;

bool is_ble_connected(){
  return ble_connected;
}

void set_ble_connected(bool connected){
  ble_connected = connected;
}


bool ble_driver_init(){
  NimBLEDevice::init(BLE_DEVICE_NAME);
  NimBLEDevice::setMTU(BLE_MTU_SIZE);

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(&ble_server_callbacks);


  // Initialize Telemetry GATT Service & Characteristic
  NimBLEService* pTelemetryService = pServer->createService(BLE_SERVICE_TELEM_UUID);
  p_gyro_char = pTelemetryService->createCharacteristic(
    BLE_CHAR_GYRO_TELEM_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  pTelemetryService->start();

  // Initialize Reserved OTA GATT Service & Characteristics (Skeleton for future OTA)
  NimBLEService* pOtaService = pServer->createService(BLE_SERVICE_OTA_UUID);
  pOtaService->createCharacteristic(
    BLE_CHAR_OTA_CONTROL_UUID,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE
  );
  pOtaService->createCharacteristic(
    BLE_CHAR_OTA_DATA_UUID,
    NIMBLE_PROPERTY::WRITE_NR
  );
  pOtaService->start();

  // Start BLE Advertising
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_TELEM_UUID);
  pAdvertising->setName(BLE_DEVICE_NAME);
  pAdvertising->start();

  Serial.println("NimBLE Driver Initialized & Advertising Started.");
  return true;
}

bool send_gyro_telemetry_notification(const uint8_t* data, size_t len){
  if(p_gyro_char == nullptr || !is_ble_connected()) return false;
  if(p_gyro_char->getSubscribedCount() == 0) return false;

  p_gyro_char->setValue(data, len);
  p_gyro_char->notify();
  return true;
}
