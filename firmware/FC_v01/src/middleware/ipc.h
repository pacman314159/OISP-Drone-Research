#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"
#include "types.h"
#include "middleware/ring_buffer.h"

// =============================================================================
// LAYER 3 MIDDLEWARE: CENTRALIZED INTER-TASK COMMUNICATION (IPC) ([Note 0002])
// =============================================================================

// System Mutex Handles (I2C Bus Synchronization Locks)
extern SemaphoreHandle_t i2c0_mutex;
extern SemaphoreHandle_t i2c1_mutex;

// Thread-Safe Inter-Task Ring Buffers
extern RingBuffer<IMUSample, GYRO_RAW_RING_BUF_SIZE> imu_ring_buffer;

// Centralized Inter-Task Communication (IPC) Initialization Routine
bool init_ipc();
