#pragma once
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// =============================================================================
// I2C BUS OPERATING MODE SELECTION
// =============================================================================
constexpr bool SYSTEM_I2C0_MODE_ASYNC   = false; // false = Synchronous, true = Asynchronous
constexpr bool SYSTEM_I2C1_MODE_ASYNC   = false; // false = Synchronous, true = Asynchronous
constexpr uint32_t I2C_FREQ_HZ          = 400000;

constexpr bool ENABLE_SIMD_ACCELERATION = true;  // Hardware SIMD Vector Math Acceleration Flag

#define DISABLE_I2C1_FOR_JTAG 1

// =============================================================================
// ESP32-S3 HARDWARE PIN DEFINITIONS
// =============================================================================
#define IMU_I2C_BUS_NUM         0 // Primary IMU Bus Selector (0 = I2C0, 1 = I2C1)

// I2C Buses Pins
constexpr int I2C0_SCL_PIN      = 14;
constexpr int I2C0_SDA_PIN      = 21;
constexpr int I2C1_SCL_PIN      = 20;
constexpr int I2C1_SDA_PIN      = 19;

// Primary Sensor IMU Bus Mapping
constexpr int IMU_SCL_PIN       = I2C0_SCL_PIN;
constexpr int IMU_SDA_PIN       = I2C0_SDA_PIN;

// Motor PWM Output Pins
constexpr int ESC_PWM1_PIN      = 4;
constexpr int ESC_PWM2_PIN      = 5;
constexpr int ESC_PWM3_PIN      = 17;
constexpr int ESC_PWM4_PIN      = 48;

// SPI Bus Pins
constexpr int SPI_MOSI_PIN      = 11;
constexpr int SPI_SCK_PIN       = 12;
constexpr int SPI_MISO_PIN      = 13;
constexpr int SPI_CS_LORA_PIN   = 6;
constexpr int SPI_CS_PERIP_PIN  = 15;
constexpr int SPI_CS_SD_PIN     = 9;

// Serial UART Pins
constexpr int UART_TX_PIN       = 8;
constexpr int UART_RX_PIN       = 18;

// RC Receiver Input Pins
constexpr int RX_PWM1_PIN       = 1; // PPM
constexpr int RX_PWM2_PIN       = 2;
constexpr int RX_PWM3_PIN       = 42;
constexpr int RX_PWM4_PIN       = 41;
constexpr int RX_PWM5_PIN       = 40;
constexpr int RX_PWM6_PIN       = 39;

// Other pins
constexpr int LORA_RST_PIN      = 7;
constexpr int SD_CARD_DET_PIN   = 3;
constexpr int IMU_MAG_DRDY_PIN  = 10;
constexpr int USER_BTN_PIN      = 38;
constexpr int RGB_LED_PIN       = 47;

// =============================================================================
// OPERATING SYSTEM ABSTRACTION LAYER (OSAL) Synchronization configs
// =============================================================================
constexpr int RING_BUF_MAX_SIZE         = 100; // Maximum allowable ring buffer size
constexpr int IMU_RAW_RING_BUF_SIZE     = 10;
constexpr uint32_t BUS_MUTEX_TIMEOUT_MS = 5;   // Global xSemaphoreTake timeout (5 ms)

// =============================================================================
// BLE CONFIGURATION & GATT SERVICE DEFINITIONS
// =============================================================================
constexpr const char* BLE_DEVICE_NAME           = "FC_v01";
constexpr uint16_t BLE_MTU_SIZE                 = 512; // bytes
constexpr uint8_t BLE_TELEM_SAMPLES_PER_PACKET = 31;  // 31 samples x 16B = 496B payload

enum TelemTimestampUnit {
  TIMESTAMP_UNIT_MICROSECONDS,
  TIMESTAMP_UNIT_MILLISECONDS
};
constexpr TelemTimestampUnit BLE_TELEM_TIMESTAMP_UNIT = TIMESTAMP_UNIT_MICROSECONDS;

enum TelemSensorSource {
  TELEM_SENSOR_GYRO  = 0,
  TELEM_SENSOR_ACCEL = 1,
  TELEM_SENSOR_MAG   = 2
};
constexpr TelemSensorSource BLE_TELEM_SENSOR_SRC = TELEM_SENSOR_GYRO;

// Telemetry GATT Service (Compatible with ble_9dof_daq / FCDatMon)
constexpr const char* BLE_SERVICE_TELEM_UUID   = "d3a9f560-8f77-4a45-b3e0-3c22d8f23c91";
constexpr const char* BLE_CHAR_GYRO_TELEM_UUID = "b4f61a92-4c35-4f93-b2e1-89a37f6a5fdd";

// Reserved BLE OTA Service (Dual-bank firmware update support)
constexpr const char* BLE_SERVICE_OTA_UUID      = "23408888-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr const char* BLE_CHAR_OTA_CONTROL_UUID = "23408889-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr const char* BLE_CHAR_OTA_DATA_UUID    = "2340888a-1f40-4cd8-9b89-ca8d45f8a5b0";

// =============================================================================
// CENTRALIZED FREERTOS TASK CONFIGURATIONS
// =============================================================================

constexpr const char* TASK_IMU_DAQ_NAME        = "IMU_DAQ";
constexpr const char* TASK_ATTITUDE_NAME       = "ATTITUDE";
constexpr const char* TASK_BARO_DAQ_NAME       = "BARO_DAQ";
constexpr const char* TASK_MAG_DAQ_NAME        = "MAG_DAQ";
constexpr const char* TASK_BLE_VEC3_TRANS_NAME = "BLE_VEC3_TRANS_TASK";
constexpr const char* TASK_LED_BLINK_NAME      = "LED_BLINK";

enum TaskCores : int32_t {
  TASK_IMU_DAQ_CORE        = tskNO_AFFINITY,
  TASK_ATTITUDE_CORE       = 1,
  TASK_BARO_DAQ_CORE       = 1,
  TASK_MAG_DAQ_CORE        = 1,
  TASK_BLE_VEC3_TRANS_CORE = tskNO_AFFINITY,
  TASK_LED_BLINK_CORE      = 1
};

enum TaskPriorities : UBaseType_t {
  TASK_IMU_DAQ_PRIORITY        = 4,
  TASK_ATTITUDE_PRIORITY       = 3,
  TASK_BARO_DAQ_PRIORITY       = 2,
  TASK_MAG_DAQ_PRIORITY        = 2,
  TASK_BLE_VEC3_TRANS_PRIORITY = 1,
  TASK_LED_BLINK_PRIORITY      = 1
};

enum TaskSizesBytes : uint32_t {
  TASK_IMU_DAQ_STACK_SIZE        = 4096,
  TASK_ATTITUDE_STACK_SIZE       = 4096,
  TASK_BARO_DAQ_STACK_SIZE       = 2048,
  TASK_MAG_DAQ_STACK_SIZE        = 2048,
  TASK_BLE_VEC3_TRANS_STACK_SIZE = 8192,
  TASK_LED_BLINK_STACK_SIZE      = 2048
};

enum TaskFrequenciesHz : uint32_t {
  TASK_IMU_DAQ_FREQ_HZ        = 500,
  TASK_ATTITUDE_FREQ_HZ       = 500,
  TASK_BARO_DAQ_FREQ_HZ       = 50,
  TASK_MAG_DAQ_FREQ_HZ        = 50,
  TASK_BLE_VEC3_TRANS_FREQ_HZ = 100,
  TASK_LED_BLINK_FREQ_HZ      = 2
};
