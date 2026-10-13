#include "app/app.h"
#include "config.h"

#include "platforms/hal/hal_i2c.h"
#include "platforms/hal/hal_pulse.h"
#include "platforms/hal/hal_spi.h"
#include "platforms/hal/hal_flash.h"

#include "middleware/ipc.h"

#include "drivers/imu/mpu6050.h"
#include "drivers/imu/hmc5883l.h"
#include "drivers/baro/bmp180.h"
#include "drivers/led/ws2812.h"
#include "drivers/telemetry/ble_driver.h"
#include "drivers/storage/sd_card_driver.h"
#include "drivers/storage/flash_storage_driver.h"

#include "core/sensor/daq_tasks.h"
#include "core/sensor/calib.h"
#include "core/math/filter_tasks.h"
#include "core/telemetry/ble_telem_tasks.h"
#include "core/blackbox/sd_logger_task.h"
#include "core/telemetry/ble_ota_tasks.h"
#include "core/interface/user_button.h"
#include "core/interface/led_task.h"
#include "app/fsm.h"

// Pure HAL Interface Accessor (Target Hardware Decoupled per [Note 0002])
static HAL_I2C& i2c0 = get_i2c0_bus();
static MPU6050 mpu(i2c0);
static HMC5883L hmc(i2c0);
static BMP180 bmp(i2c0);

static HAL_PulseTX& pulse_tx0 = get_pulse_tx0();
static WS2812 status_led(pulse_tx0);

static HAL_SPI& spi_bus = get_spi_bus();
static SDCardDriver sd_driver(spi_bus, SPI_CS_SD_PIN, SD_CARD_DET_PIN);

static HAL_Flash& flash_hal = get_flash_storage();
static FlashStorageDriver flash_driver(flash_hal);

void app_suspend_all_tasks(){
  for(uint32_t i = 0; i < TASK_COUNT; ++i)
    if(i != TASK_BLE_OTA_ID && app_task_handles[i] != nullptr)
      vTaskSuspend(app_task_handles[i]);
}

void app_start(){
  if(xSemaphoreTake(i2c0_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
    mpu.init();
    mpu.enable_bypass();
    hmc.init();
    bmp.init();
    xSemaphoreGive(i2c0_mutex);
  }

  status_led.init(RGB_LED_PIN);
  status_led.set_brightness(RGB_LED_DEFAULT_BRIGHTNESS);

  ble_driver_init();

  spi_bus.init(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN);

  user_button_init();

  if(flash_driver.init("nvs")){
    bool calib_loaded = calib_init(&flash_driver);
    if(calib_loaded){
      const SensorCalibrationData& cal = calib_get_data();
      Serial.printf("[BOOT] Calibration loaded: Gyro Bias [%.3f, %.3f, %.3f], Accel Scale [%.3f, %.3f, %.3f], Mag Bias [%.3f, %.3f, %.3f], Mag B0: %.3f G\n",
                    cal.gyro_bias.x, cal.gyro_bias.y, cal.gyro_bias.z,
                    cal.accel_scale.x, cal.accel_scale.y, cal.accel_scale.z,
                    cal.mag_bias.x, cal.mag_bias.y, cal.mag_bias.z,
                    cal.mag_field_ref);
    }else{
      Serial.println("[BOOT] No valid calibration found in Flash. Using unity defaults.");
    }
  }else{
    Serial.println("[BOOT] Flash NVS initialization failed!");
  }

  if constexpr(ACTIVE_SD_LOG_PACKETS_MASK != LOG_MASK_NONE){
    if(xSemaphoreTake(spi_bus_mutex, pdMS_TO_TICKS(BUS_MUTEX_TIMEOUT_MS)) == pdTRUE){
      if(sd_driver.init()){
        char filename[32];
        uint16_t session_id = 0;
        if(sd_driver.open_next_log_file(filename, sizeof(filename), session_id)){
          LogFileHeader header = {};
          memcpy(header.magic, "FCLG", 4);
          header.session_id              = session_id;
          strncpy(header.board_name, BOARD_IDENTIFIER, sizeof(header.board_name) - 1);
          header.boot_timestamp_us       = static_cast<uint32_t>(micros());
          header.registered_packets_mask = ACTIVE_SD_LOG_PACKETS_MASK;
          header.limit_policy            = static_cast<uint8_t>(LOG_LIMIT_POLICY);
          header.max_file_size_bytes     = LOG_MAX_FILE_SIZE_BYTES;
          header.circular_head_sector    = 1;
          sd_driver.write_header(header);
        }
      }
      xSemaphoreGive(spi_bus_mutex);
    }
  }

  fsm_init();

  xTaskCreatePinnedToCore(status_led_task, TASK_STATUS_LED_NAME, TASK_STATUS_LED_STACK_SIZE, &status_led, TASK_STATUS_LED_PRIORITY, &app_task_handles[TASK_STATUS_LED_ID], TASK_STATUS_LED_CORE);
  xTaskCreatePinnedToCore(fsm_task, TASK_FSM_NAME, TASK_FSM_STACK_SIZE, nullptr, TASK_FSM_PRIORITY, &app_task_handles[TASK_FSM_ID], TASK_FSM_CORE);
  xTaskCreatePinnedToCore(ble_ota_task, TASK_BLE_OTA_NAME, TASK_BLE_OTA_STACK_SIZE, nullptr, TASK_BLE_OTA_PRIORITY, &app_task_handles[TASK_BLE_OTA_ID], TASK_BLE_OTA_CORE);
  xTaskCreatePinnedToCore(accel_gyro_daq_task, TASK_ACCEL_GYRO_DAQ_NAME, TASK_ACCEL_GYRO_DAQ_STACK_SIZE, &mpu, TASK_ACCEL_GYRO_DAQ_PRIORITY, &app_task_handles[TASK_ACCEL_GYRO_DAQ_ID], TASK_ACCEL_GYRO_DAQ_CORE);
  xTaskCreatePinnedToCore(mag_daq_task, TASK_MAG_DAQ_NAME, TASK_MAG_DAQ_STACK_SIZE, &hmc, TASK_MAG_DAQ_PRIORITY, &app_task_handles[TASK_MAG_DAQ_ID], TASK_MAG_DAQ_CORE);
  xTaskCreatePinnedToCore(baro_daq_task, TASK_BARO_DAQ_NAME, TASK_BARO_DAQ_STACK_SIZE, &bmp, TASK_BARO_DAQ_PRIORITY, &app_task_handles[TASK_BARO_DAQ_ID], TASK_BARO_DAQ_CORE);
  xTaskCreatePinnedToCore(accel_gyro_lpf_task, TASK_ACCEL_GYRO_LPF_NAME, TASK_ACCEL_GYRO_LPF_STACK_SIZE, nullptr, TASK_ACCEL_GYRO_LPF_PRIORITY, &app_task_handles[TASK_ACCEL_GYRO_LPF_ID], TASK_ACCEL_GYRO_LPF_CORE);
  xTaskCreatePinnedToCore(mag_lpf_task, TASK_MAG_LPF_NAME, TASK_MAG_LPF_STACK_SIZE, nullptr, TASK_MAG_LPF_PRIORITY, &app_task_handles[TASK_MAG_LPF_ID], TASK_MAG_LPF_CORE);

  if constexpr(ACTIVE_BLE_TELEM_MASK != BLE_TELEM_MASK_NONE)
    xTaskCreatePinnedToCore(ble_telem_task, TASK_BLE_TELEM_NAME, TASK_BLE_TELEM_STACK_SIZE, nullptr, TASK_BLE_TELEM_PRIORITY, &app_task_handles[TASK_BLE_TELEM_ID], TASK_BLE_TELEM_CORE);

  if constexpr(ACTIVE_SD_LOG_PACKETS_MASK != LOG_MASK_NONE){
    if(sd_driver.is_open())
      xTaskCreatePinnedToCore(sd_logger_task, TASK_SD_LOGGER_NAME, TASK_SD_LOGGER_STACK_SIZE, &sd_driver, TASK_SD_LOGGER_PRIORITY, &app_task_handles[TASK_SD_LOGGER_ID], TASK_SD_LOGGER_CORE);
  }
}



