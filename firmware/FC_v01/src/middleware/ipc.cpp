#include "middleware/ipc.h"

SemaphoreHandle_t i2c0_mutex = nullptr;
SemaphoreHandle_t i2c1_mutex = nullptr;

RingBuffer<Vec3<int16_t>, GYRO_RAW_RING_BUF_SIZE> accel_raw_msb;
RingBuffer<Vec3<int16_t>, GYRO_RAW_RING_BUF_SIZE> gyro_raw_msb;
RingBuffer<Vec3<int16_t>, GYRO_RAW_RING_BUF_SIZE> mag_raw_msb;
RingBuffer<Vec3<int16_t>, GYRO_RAW_RING_BUF_SIZE> pressure_raw_msb;

bool init_ipc(){
  if(i2c0_mutex == nullptr)
    i2c0_mutex = xSemaphoreCreateMutex();

  if(i2c1_mutex == nullptr)
    i2c1_mutex = xSemaphoreCreateMutex();

  return (i2c0_mutex != nullptr && i2c1_mutex != nullptr);
}
