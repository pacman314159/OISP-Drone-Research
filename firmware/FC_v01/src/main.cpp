#include <Arduino.h>
#include "config.h"
#include "freertos/projdefs.h"
#include "platforms/hal/hal_i2c.h"
#include "middleware/ipc.h"
#include "app/app.h"

extern "C" void app_main(){
  initArduino();

  init_ipc();

  Serial.begin(115200);
  HAL_I2C& i2c0 = get_i2c0_bus();

  i2c0.init(I2C_FREQ_HZ);
  vTaskDelay(pdMS_TO_TICKS(50));

  app_start();
}
