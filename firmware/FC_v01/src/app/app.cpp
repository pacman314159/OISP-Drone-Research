#include "app/app.h"
#include "config.h"
#include "platforms/hal/hal_i2c.h"
#include "middleware/ipc.h"
#include "drivers/imu/mpu6050.h"
#include "drivers/driver_tasks.h"

// Pure HAL Interface Accessor (Target Hardware Decoupled per [Note 0002])
static HAL_I2C& i2c0 = get_i2c0_bus();
static MPU6050 mpu(i2c0);

void app_start(){
  // Initialize Layer 2 MPU6050 Sensor under i2c0_mutex guard
  if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
    mpu.init();
    xSemaphoreGive(i2c0_mutex);
  }

  // Spawn 500 Hz IMU DAQ Task (Layer 2 task, configured via [Note 0002] config.h enums)
  xTaskCreatePinnedToCore(
    accel_gyro_daq_task,
    "IMU_DAQ",
    TASK_IMU_DAQ_STACK_SIZE,
    &mpu,
    TASK_PRIORITY_REALTIME,
    nullptr,
    TASK_IMU_DAQ_CORE
  );
}
