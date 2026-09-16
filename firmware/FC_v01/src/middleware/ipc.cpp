#include "middleware/ipc.h"

// Instantiate Global FreeRTOS Mutex Handles
SemaphoreHandle_t i2c0_mutex = nullptr;
SemaphoreHandle_t i2c1_mutex = nullptr;

// Instantiate Global Thread-Safe Static IMU Ring Buffer
RingBuffer<IMUSample, GYRO_RAW_RING_BUF_SIZE> imu_ring_buffer;

bool init_ipc(){
  if(i2c0_mutex == nullptr)
    i2c0_mutex = xSemaphoreCreateMutex();

  if(i2c1_mutex == nullptr)
    i2c1_mutex = xSemaphoreCreateMutex();

  return (i2c0_mutex != nullptr && i2c1_mutex != nullptr);
}
