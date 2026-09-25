#include "app/app.h"
#include "config.h"

#include "platforms/hal/hal_i2c.h"

#include "middleware/ipc.h"

#include "drivers/imu/mpu6050.h"
#include "drivers/imu/hmc5883l.h"
#include "drivers/telemetry/ble_driver.h"

#include "core/daq/daq_tasks.h"
#include "core/telemetry/telem_tasks.h"

// Pure HAL Interface Accessor (Target Hardware Decoupled per [Note 0002])
static HAL_I2C& i2c0 = get_i2c0_bus();
static MPU6050 mpu(i2c0);
static HMC5883L hmc(i2c0);

void app_start(){
  if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
    mpu.init();
    mpu.enable_bypass();
    hmc.init();
    xSemaphoreGive(i2c0_mutex);
  }

  ble_driver_init();

  xTaskCreatePinnedToCore(
    accel_gyro_daq_task,
    TASK_ACCEL_GYRO_DAQ_NAME,
    TASK_ACCEL_GYRO_DAQ_STACK_SIZE,
    &mpu,
    TASK_ACCEL_GYRO_DAQ_PRIORITY,
    nullptr,
    TASK_ACCEL_GYRO_DAQ_CORE
  );

  xTaskCreatePinnedToCore(
    mag_daq_task,
    TASK_MAG_DAQ_NAME,
    TASK_MAG_DAQ_STACK_SIZE,
    &hmc,
    TASK_MAG_DAQ_PRIORITY,
    nullptr,
    TASK_MAG_DAQ_CORE
  );

  xTaskCreatePinnedToCore(
    ble_vec3_trans_task,
    TASK_BLE_VEC3_TRANS_NAME,
    TASK_BLE_VEC3_TRANS_STACK_SIZE,
    nullptr,
    TASK_BLE_VEC3_TRANS_PRIORITY,
    nullptr,
    TASK_BLE_VEC3_TRANS_CORE
  );
}

