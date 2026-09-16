#pragma once
#include <cstdint>

// =============================================================================
// I2C BUS OPERATING MODE SELECTION
// =============================================================================
constexpr bool SYSTEM_I2C0_MODE_ASYNC = false; // false = Synchronous, true = Asynchronous
constexpr bool SYSTEM_I2C1_MODE_ASYNC = false; // false = Synchronous, true = Asynchronous
constexpr uint32_t I2C_FREQ_HZ        = 400000;

// Set to 1 to reserve GPIO19 (D-) and GPIO20 (D+) for built-in USB JTAG debugging (disables I2C1 bus)
#define DISABLE_I2C1_FOR_JTAG 1

// =============================================================================
// ESP32-S3 HARDWARE PIN DEFINITIONS (Synchronized from FC_v01 Schematic)
// =============================================================================
#define IMU_I2C_BUS_NUM         0 // Primary IMU Bus Selector (0 = I2C0, 1 = I2C1)

// I2C Buses Pins
constexpr int I2C0_SCL_PIN      = 14;
constexpr int I2C0_SDA_PIN      = 21;
constexpr int I2C1_SCL_PIN      = 20;
constexpr int I2C1_SDA_PIN      = 19;

// Primary Sensor IMU Bus Mapping (Gyroscope, Accelerometer, Magnetometer)
constexpr int IMU_SCL_PIN       = I2C0_SCL_PIN;
constexpr int IMU_SDA_PIN       = I2C0_SDA_PIN;

// Motor PWM Output Pins (ESC_PWM1 to ESC_PWM4)
constexpr int ESC_PWM1_PIN      = 4; 
constexpr int ESC_PWM2_PIN      = 5; 
constexpr int ESC_PWM3_PIN      = 17;
constexpr int ESC_PWM4_PIN      = 48;

// SPI Bus Pins (Ra-02 LoRa Module & MicroSD Card Socket)
constexpr int SPI_MOSI_PIN      = 11;
constexpr int SPI_SCK_PIN       = 12;
constexpr int SPI_MISO_PIN      = 13;
constexpr int SPI_CS_LORA_PIN   = 6; 
constexpr int SPI_CS_PERIP_PIN  = 15;
constexpr int SPI_CS_SD_PIN     = 9;

// Telemetry Serial UART Pins
constexpr int UART_TX_PIN       = 8;
constexpr int UART_RX_PIN       = 18;

// RC Receiver Input Pins
constexpr int RX_PWM1_PIN       = 1; // PPM
constexpr int RX_PWM2_PIN       = 2; 
constexpr int RX_PWM3_PIN       = 42;
constexpr int RX_PWM4_PIN       = 41;
constexpr int RX_PWM5_PIN       = 40;
constexpr int RX_PWM6_PIN       = 39;

// Other pin mappings
constexpr int LORA_RST_PIN      = 7; 
constexpr int SD_CARD_DET_PIN   = 3; 
constexpr int IMU_MAG_DRDY_PIN  = 10; 
constexpr int USER_BTN_PIN      = 38;
constexpr int RGB_LED_PIN       = 47;

// =============================================================================
// OPERATING SYSTEM ABSTRACTION LAYER (OSAL) Synchronization configs
// =============================================================================
constexpr int RING_BUF_MAX_SIZE           = 100; // Maximum allowable ring buffer size

constexpr int GYRO_RAW_RING_BUF_SIZE      = 10;
constexpr int ACCEL_RAW_RING_BUF_SIZE     = 10;
constexpr int MAG_RAW_RING_BUF_SIZE       = 10;

constexpr uint32_t BUS_MUTEX_TIMEOUT_MS   = 5;  // Global xSemaphoreTake timeout (5 ms)


// =============================================================================
// CENTRALIZED FREERTOS TASK CONFIGURATIONS (Enforced by [Note 0002])
// =============================================================================

// Rule 1: Strict 4-Level RTOS Priority Architecture
enum TaskPriorities : uint8_t {
  TASK_PRIORITY_LOW      = 1, // Telemetry, Housekeeping
  TASK_PRIORITY_MED      = 2, // Barometer, Magnetometer DAQ
  TASK_PRIORITY_HIGH     = 3, // Attitude Rate & Control Loops
  TASK_PRIORITY_REALTIME = 4  // 500 Hz IMU DAQ Loop
};

// Rule 2: Structured Enum Task Parameters
enum TaskNames : uint8_t {
  TASK_IMU_DAQ_NAME_ID   = 0,
  TASK_ATTITUDE_NAME_ID  = 1,
  TASK_BARO_DAQ_NAME_ID  = 2,
  TASK_MAG_DAQ_NAME_ID   = 3
};

enum TaskSizes : uint32_t {
  TASK_IMU_DAQ_STACK_SIZE = 4096, // Stack size in bytes
  TASK_ATTITUDE_STACK_SIZE= 4096,
  TASK_BARO_DAQ_STACK_SIZE= 2048,
  TASK_MAG_DAQ_STACK_SIZE = 2048
};

enum TaskFrequencies : uint16_t {
  TASK_IMU_DAQ_FREQ_HZ   = 500,
  TASK_ATTITUDE_FREQ_HZ  = 250,
  TASK_MAG_DAQ_FREQ_HZ    = 75,
  TASK_BARO_DAQ_FREQ_HZ   = 50
};

constexpr int TASK_IMU_DAQ_CORE = 1; // Pinned to Core 1

