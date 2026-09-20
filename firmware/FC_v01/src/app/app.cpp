#include "app/app.h"
#include "config.h"

#include "platforms/hal/hal_i2c.h"
#include "platforms/hal/hal_rgb_led.h"

#include "middleware/ipc.h"

#include "drivers/imu/mpu6050.h"
#include "drivers/telemetry/ble_driver.h"
#include "drivers/led/ws2812b.h"

#include "core/daq/daq_tasks.h"
#include "core/telemetry/telem_tasks.h"

// Pure HAL Interface Accessor (Target Hardware Decoupled per [Note 0002])
static HAL_I2C& i2c0 = get_i2c0_bus();
static MPU6050 mpu(i2c0);
static HAL_RGB_LED& rgb_led_hal = get_rgb_led();
static WS2812B status_led(rgb_led_hal);

void app_start(){
  status_led.init(RGB_LED_PIN);

  if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
    mpu.init();
    xSemaphoreGive(i2c0_mutex);
  }

  ble_driver_init();

  xTaskCreatePinnedToCore(
    accel_gyro_daq_task,
    TASK_IMU_DAQ_NAME,
    TASK_IMU_DAQ_STACK_SIZE,
    &mpu,
    TASK_IMU_DAQ_PRIORITY,
    nullptr,
    TASK_IMU_DAQ_CORE
  );

  xTaskCreatePinnedToCore(
    ble_vec3_trans_task,
    TASK_BLE_VEC3_TRANS_NAME,
    TASK_BLE_VEC3_TRANS_STACK_SIZE,
    const_cast<TelemSensorSource*>(&BLE_TELEM_SENSOR_SRC),
    TASK_BLE_VEC3_TRANS_PRIORITY,
    nullptr,
    TASK_BLE_VEC3_TRANS_CORE
  );

  xTaskCreatePinnedToCore(
    led_blink_task,
    TASK_LED_BLINK_NAME,
    TASK_LED_BLINK_STACK_SIZE,
    nullptr,
    TASK_LED_BLINK_PRIORITY,
    nullptr,
    TASK_LED_BLINK_CORE
  );
}

