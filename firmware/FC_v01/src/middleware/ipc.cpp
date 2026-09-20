#include <Arduino.h>
#include "middleware/ipc.h"
#include "drivers/telemetry/ble_driver.h"

SemaphoreHandle_t i2c0_mutex = nullptr;
SemaphoreHandle_t i2c1_mutex = nullptr;

RingBuffer<Vec3<float>, IMU_RAW_RING_BUF_SIZE> accel_raw;
RingBuffer<Vec3<float>, IMU_RAW_RING_BUF_SIZE> gyro_raw;
RingBuffer<uint32_t, IMU_RAW_RING_BUF_SIZE> accel_gyro_timestamp;
RingBuffer<Vec3<float>, IMU_RAW_RING_BUF_SIZE> mag_raw;
RingBuffer<Vec3<float>, IMU_RAW_RING_BUF_SIZE> pressure_raw;

QueueHandle_t gyro_telemetry_queue = nullptr;
FcBleServerCallbacks ble_server_callbacks;


void FcBleServerCallbacks::onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc){
  set_ble_connected(true);
  if(desc != nullptr)
    pServer->updateConnParams(desc->conn_handle, 6, 12, 0, 200);
  Serial.println("BLE Client Connected!");
}

void FcBleServerCallbacks::onDisconnect(NimBLEServer* pServer){
  set_ble_connected(false);
  Serial.println("BLE Client Disconnected!");
  NimBLEDevice::startAdvertising();
}

bool init_ipc(){
  if(i2c0_mutex == nullptr)
    i2c0_mutex = xSemaphoreCreateMutex();

  if(i2c1_mutex == nullptr)
    i2c1_mutex = xSemaphoreCreateMutex();

  if(gyro_telemetry_queue == nullptr)
    gyro_telemetry_queue = xQueueCreate(60, sizeof(Vec3<int16_t>));

  return (i2c0_mutex != nullptr && i2c1_mutex != nullptr && gyro_telemetry_queue != nullptr);
}


