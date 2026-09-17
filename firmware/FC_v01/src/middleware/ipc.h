#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"
#include "middleware/ring_buffer.h"
#include "src/core/math/matrix.h"

// =============================================================================
// LAYER 3 MIDDLEWARE: CENTRALIZED INTER-TASK COMMUNICATION (IPC) ([Note 0002])
// =============================================================================

// System Mutex Handles (I2C Bus Synchronization Locks)
extern SemaphoreHandle_t i2c0_mutex;
extern SemaphoreHandle_t i2c1_mutex;

// Thread-Safe Inter-Task Ring Buffers
extern RingBuffer<Vec3<int16_t>, GYRO_RAW_RING_BUF_SIZE> accel_raw_msb;
extern RingBuffer<Vec3<int16_t>, GYRO_RAW_RING_BUF_SIZE> gyro_raw_msb;
extern RingBuffer<Vec3<int16_t>, GYRO_RAW_RING_BUF_SIZE> mag_raw_msb;
extern RingBuffer<Vec3<int16_t>, GYRO_RAW_RING_BUF_SIZE> pressure_raw_msb;

// Centralized Inter-Task Communication (IPC) Initialization Routine
bool init_ipc();
