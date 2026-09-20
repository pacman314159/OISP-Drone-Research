#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "middleware/ring_buffer.h"
#include "src/core/math/matrix.h"

// =============================================================================
// LAYER 3 MIDDLEWARE: CENTRALIZED INTER-TASK COMMUNICATION (IPC) ([Note 0002])
// =============================================================================

// System Mutex Handles (I2C Bus Synchronization Locks)
extern SemaphoreHandle_t i2c0_mutex;
extern SemaphoreHandle_t i2c1_mutex;
extern QueueHandle_t gyro_telemetry_queue;

// Thread-Safe Inter-Task Ring Buffers
extern RingBuffer<Vec3<float>, IMU_RAW_RING_BUF_SIZE> accel_raw;
extern RingBuffer<Vec3<float>, IMU_RAW_RING_BUF_SIZE> gyro_raw;
extern RingBuffer<uint32_t, IMU_RAW_RING_BUF_SIZE> accel_gyro_timestamp;
extern RingBuffer<Vec3<float>, IMU_RAW_RING_BUF_SIZE> mag_raw;
extern RingBuffer<Vec3<float>, IMU_RAW_RING_BUF_SIZE> pressure_raw;


// Layer 3 Middleware Callbacks for NimBLE Server Events ([Note 0001])
class FcBleServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override;
  void onDisconnect(NimBLEServer* pServer) override;
};

extern FcBleServerCallbacks ble_server_callbacks;

// Centralized Inter-Task Communication (IPC) Initialization Routine
bool init_ipc();


