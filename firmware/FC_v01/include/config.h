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

constexpr int ESC_PWM1_PIN      = 4;
constexpr int ESC_PWM2_PIN      = 3;
constexpr int ESC_PWM3_PIN      = 40;
constexpr int ESC_PWM4_PIN      = 1;

// SPI Bus Pins
constexpr int SPI_MOSI_PIN      = 11;
constexpr int SPI_SCK_PIN       = 12;
constexpr int SPI_MISO_PIN      = 13;
constexpr int SPI_CS_LORA_PIN   = 39;
constexpr int SPI_CS_PERIP_PIN  = 47;
constexpr int SPI_CS_SD_PIN     = 8;

// Serial UART Pins
constexpr int UART_TX_PIN       = 9;
constexpr int UART_RX_PIN       = 10;

// RC Receiver Input Pins
constexpr int RX_PWM1_PIN       = 5; // PPM
constexpr int RX_PWM2_PIN       = 6;
constexpr int RX_PWM3_PIN       = 7;
constexpr int RX_PWM4_PIN       = 15;
constexpr int RX_PWM5_PIN       = 16;
constexpr int RX_PWM6_PIN       = 17;

// Other pins
constexpr int USER_BTN_PIN      = 38;
constexpr int LORA_RST_PIN      = 41;
constexpr int SD_CARD_DET_PIN   = 18;
constexpr int IMU_MAG_DRDY_PIN  = 42;
constexpr int RGB_LED_PIN       = 2;  // WS2812 Single RGB Status LED on PCB v02 (GPIO2)

// =============================================================================
// OPERATING SYSTEM ABSTRACTION LAYER (OSAL) Synchronization configs
// =============================================================================
constexpr int RING_BUF_MAX_SIZE         = 100; // Maximum allowable ring buffer size
constexpr int IMU_RAW_RING_BUF_SIZE     = 10;
constexpr int IMU_FILT_RING_BUF_SIZE    = 10;
constexpr uint32_t BUS_MUTEX_TIMEOUT_MS = 20;  // Global xSemaphoreTake timeout (20 ms, must exceed Wire timeout + max I2C transaction)

// =============================================================================
// BLE CONFIGURATION & GATT SERVICE DEFINITIONS
// =============================================================================
// constexpr const char* BLE_DEVICE_NAME           = "FC_v01";
constexpr const char* BLE_DEVICE_NAME           = "FC_v02";
constexpr uint16_t BLE_MTU_SIZE                 = 512; // bytes
constexpr uint8_t BLE_GYRO_SAMPLES_PER_PACKET      = 31;  // 31 samples x 16B = 496B payload
constexpr uint8_t BLE_ACCEL_SAMPLES_PER_PACKET     = 31;  // 31 samples x 16B = 496B payload
constexpr uint8_t BLE_MAG_SAMPLES_PER_PACKET       = 31;  // 31 samples x 16B = 496B payload
constexpr uint8_t BLE_BARO_SAMPLES_PER_PACKET      = 63;  // 63 samples x 8B = 504B payload
constexpr uint8_t BLE_ATT_QUAT_SAMPLES_PER_PACKET  = 25;  // 25 samples x 20B = 500B payload
constexpr uint8_t BLE_ATT_EULER_SAMPLES_PER_PACKET = 31;  // 31 samples x 16B = 496B payload
constexpr uint32_t BLE_TELEM_MIN_PACKET_FREQ_HZ    = 10;

enum TelemTimestampUnit {
  TIMESTAMP_UNIT_MICROSECONDS,
  TIMESTAMP_UNIT_MILLISECONDS
};
constexpr TelemTimestampUnit BLE_TELEM_TIMESTAMP_UNIT = TIMESTAMP_UNIT_MICROSECONDS;

// Telemetry GATT Service & Characteristics (Fresh 128-bit UUID Series)
constexpr const char* BLE_SERVICE_TELEM_UUID        = "7c9a41b2-e2d7-4d9e-99f6-1e35a90d81f2";
constexpr const char* BLE_CHAR_GYRO_RAW_UUID        = "4a8f9b2d-c13e-4f76-8b21-6d9a5e8c3b40";
constexpr const char* BLE_CHAR_GYRO_FILT_UUID       = "f3c2a1e8-7b4d-4e92-95a6-0c8f1e2d3b4a";
constexpr const char* BLE_CHAR_ACCEL_RAW_UUID       = "8b7e2d14-3c6a-4f91-8e52-a9b0c1d2e3f4";
constexpr const char* BLE_CHAR_ACCEL_FILT_UUID      = "1e9a3b5c-6d8f-4a72-b1c3-2e4d5f6a7b8c";
constexpr const char* BLE_CHAR_MAG_RAW_UUID         = "9d2c4b6a-8e1f-4c53-a7b2-3e5f7a9b1c2d";
constexpr const char* BLE_CHAR_MAG_FILT_UUID        = "2b4c6e8a-0f1d-4a3b-9c5e-7a8b9c0d1e2f";
constexpr const char* BLE_CHAR_BARO_RAW_UUID        = "d4e7a2b9-3c81-4f5e-9a62-8b1e4c7d0f3a";
constexpr const char* BLE_CHAR_BARO_FILT_UUID       = "c4a6e8b1-3d5f-4a7c-8e2b-0f1a3c5e7b9d";
constexpr const char* BLE_CHAR_ATT_QUAT_UUID        = "0d3b6a9c-2e4f-4a8b-9d1e-5f7a9c1b3e5f";
constexpr const char* BLE_CHAR_ATT_EULER_UUID       = "6e1c4a8b-0f2d-4e5a-9b3c-7d9a1e3f5b7c";

// BLE OTA Service (Dual-bank firmware update support)
constexpr const char* BLE_SERVICE_OTA_UUID      = "23408888-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr const char* BLE_CHAR_OTA_CONTROL_UUID = "23408889-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr const char* BLE_CHAR_OTA_DATA_UUID    = "2340888a-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr uint32_t OTA_BOOT_WINDOW_TIMEOUT_MS   = 10000; // Boot listening window: 10000 ms (10s)
constexpr size_t   OTA_DATA_QUEUE_DEPTH          = 32;    // Pipelined OTA queue depth (~16.5 KB SRAM, reclaimed after boot window)

// =============================================================================
// BLE TELEMETRY BITMASK & SINGLE SOURCE-OF-TRUTH
// =============================================================================
enum BleTelemBitmask : uint16_t {
  BLE_TELEM_MASK_NONE       = 0,
  BLE_TELEM_MASK_GYRO_RAW   = (1 << 0),
  BLE_TELEM_MASK_GYRO_FILT  = (1 << 1),
  BLE_TELEM_MASK_ACCEL_RAW  = (1 << 2),
  BLE_TELEM_MASK_ACCEL_FILT = (1 << 3),
  BLE_TELEM_MASK_MAG_RAW    = (1 << 4),
  BLE_TELEM_MASK_MAG_FILT   = (1 << 5),
  BLE_TELEM_MASK_BARO_RAW   = (1 << 6),
  BLE_TELEM_MASK_BARO_FILT  = (1 << 7),
  BLE_TELEM_MASK_ATT_QUAT   = (1 << 8),
  BLE_TELEM_MASK_ATT_EULER  = (1 << 9)
};

// Multi-line commentable/uncommentable BLE telemetry mask interface
constexpr uint16_t ACTIVE_BLE_TELEM_MASK =
    BLE_TELEM_MASK_NONE
    | BLE_TELEM_MASK_GYRO_RAW
    | BLE_TELEM_MASK_ACCEL_RAW
    | BLE_TELEM_MASK_GYRO_FILT
    // | BLE_TELEM_MASK_ACCEL_FILT
    | BLE_TELEM_MASK_MAG_RAW
    // | BLE_TELEM_MASK_MAG_FILT
    // | BLE_TELEM_MASK_BARO_RAW
    // | BLE_TELEM_MASK_BARO_FILT
    // | BLE_TELEM_MASK_ATT_QUAT
    // | BLE_TELEM_MASK_ATT_EULER
    ;

// =============================================================================
// CENTRALIZED FREERTOS TASK CONFIGURATIONS
// =============================================================================

constexpr const char* TASK_ACCEL_GYRO_DAQ_NAME       = "ACCEL_GYRO_DAQ";
constexpr const char* TASK_ACCEL_GYRO_LPF_NAME       = "ACCEL_GYRO_LPF";
constexpr const char* TASK_BARO_DAQ_NAME             = "BARO_DAQ";
constexpr const char* TASK_MAG_DAQ_NAME              = "MAG_DAQ";
constexpr const char* TASK_MAG_LPF_NAME              = "MAG_LPF";
constexpr const char* TASK_BLE_TELEM_NAME            = "BLE_TELEM";
constexpr const char* TASK_SD_LOGGER_NAME            = "SD_LOGGER";
constexpr const char* TASK_BLE_OTA_NAME              = "BLE_OTA";
constexpr const char* TASK_FSM_NAME                  = "FSM_MAIN";
constexpr const char* TASK_STATUS_LED_NAME           = "STATUS_LED";
constexpr const char* TASK_CALIB_GYRO_NAME           = "CALIB_GYRO";
constexpr const char* TASK_CALIB_ACCEL_NAME          = "CALIB_ACCEL";
constexpr const char* TASK_CALIB_MAG_NAME            = "CALIB_MAG";
constexpr const char* TASK_CALIB_BARO_NAME           = "CALIB_BARO";

// =============================================================================
// SD CARD & SPI LOGGING CONFIGURATION
// =============================================================================
constexpr uint32_t SPI_BUS_DEFAULT_FREQ_HZ = 10000000;
constexpr uint32_t SD_SPI_FREQ_HZ          = 20000000;
constexpr const char* SD_MOUNT_POINT       = "/sd";
constexpr const char* BOARD_IDENTIFIER     = "esp32-s3-fc-v02-n8r8";

enum LogPacketType : uint8_t {
  PKT_TYPE_IMU_RAW   = 0x01,
  PKT_TYPE_IMU_FILT  = 0x02,
  PKT_TYPE_MAG_RAW   = 0x03,
  PKT_TYPE_MAG_FILT  = 0x04,
  PKT_TYPE_BARO_RAW  = 0x05,
  PKT_TYPE_BARO_FILT = 0x06,
  PKT_TYPE_ATT_QUAT  = 0x07,
  PKT_TYPE_ATT_EULER = 0x08
};

enum LogPacketBitmask : uint16_t {
  LOG_MASK_NONE      = 0,
  LOG_MASK_IMU_RAW   = (1 << 0),
  LOG_MASK_IMU_FILT  = (1 << 1),
  LOG_MASK_MAG_RAW   = (1 << 2),
  LOG_MASK_MAG_FILT  = (1 << 3),
  LOG_MASK_BARO_RAW  = (1 << 4),
  LOG_MASK_BARO_FILT = (1 << 5),
  LOG_MASK_ATT_QUAT  = (1 << 6),
  LOG_MASK_ATT_EULER = (1 << 7)
};

// Multi-line commentable/uncommentable SD logging mask interface
constexpr uint16_t ACTIVE_SD_LOG_PACKETS_MASK =
    LOG_MASK_NONE
    // | LOG_MASK_IMU_RAW
    // | LOG_MASK_IMU_FILT
    | LOG_MASK_MAG_RAW
    // | LOG_MASK_MAG_FILT
    // | LOG_MASK_BARO_RAW
    // | LOG_MASK_BARO_FILT
    // | LOG_MASK_ATT_QUAT
    // | LOG_MASK_ATT_EULER
    ;

enum LogLimitMode : uint8_t {
  LOG_LIMIT_STOP_ON_SIZE       = 0,
  LOG_LIMIT_CIRCULAR_OVERWRITE = 1
};

constexpr LogLimitMode LOG_LIMIT_POLICY     = LOG_LIMIT_STOP_ON_SIZE;
constexpr uint32_t LOG_MAX_FILE_SIZE_BYTES = 50 * 1024 * 1024;
constexpr uint32_t LOG_SECTOR_TIMEOUT_MS         = 100;
constexpr uint32_t LOG_METADATA_SYNC_PERIOD_MS   = 1000;

enum TaskIDs : uint32_t {
  TASK_ACCEL_GYRO_DAQ_ID = 0,
  TASK_ACCEL_GYRO_LPF_ID,
  TASK_BARO_DAQ_ID,
  TASK_MAG_DAQ_ID,
  TASK_MAG_LPF_ID,
  TASK_BLE_TELEM_ID,
  TASK_SD_LOGGER_ID,
  TASK_BLE_OTA_ID,
  TASK_FSM_ID,
  TASK_STATUS_LED_ID,
  TASK_CALIB_GYRO_ID,
  TASK_CALIB_ACCEL_ID,
  TASK_CALIB_MAG_ID,
  TASK_CALIB_BARO_ID,
  TASK_COUNT
};

enum TaskCores : int32_t {
  TASK_ACCEL_GYRO_DAQ_CORE       = tskNO_AFFINITY,
  TASK_ACCEL_GYRO_LPF_CORE       = tskNO_AFFINITY,
  TASK_BARO_DAQ_CORE             = tskNO_AFFINITY,
  TASK_MAG_DAQ_CORE              = tskNO_AFFINITY,
  TASK_MAG_LPF_CORE              = tskNO_AFFINITY,
  TASK_BLE_TELEM_CORE            = tskNO_AFFINITY,
  TASK_SD_LOGGER_CORE            = tskNO_AFFINITY,
  TASK_BLE_OTA_CORE              = tskNO_AFFINITY,
  TASK_FSM_CORE                  = tskNO_AFFINITY,
  TASK_STATUS_LED_CORE           = tskNO_AFFINITY,
  TASK_CALIB_GYRO_CORE           = tskNO_AFFINITY,
  TASK_CALIB_ACCEL_CORE          = tskNO_AFFINITY,
  TASK_CALIB_MAG_CORE            = tskNO_AFFINITY,
  TASK_CALIB_BARO_CORE           = tskNO_AFFINITY
};

enum TaskPriorities : UBaseType_t {
  TASK_ACCEL_GYRO_DAQ_PRIORITY       = 3, // 500 Hz Realtime DAQ Loop
  TASK_ACCEL_GYRO_LPF_PRIORITY       = 3, // 500 Hz Realtime LPF Loop
  TASK_BARO_DAQ_PRIORITY             = 2, // Medium priority sensor DAQ
  TASK_MAG_DAQ_PRIORITY              = 2, // Medium priority sensor DAQ
  TASK_MAG_LPF_PRIORITY              = 2, // Medium priority sensor LPF
  TASK_BLE_TELEM_PRIORITY            = 1, // Low priority unified telemetry dispatcher task
  TASK_SD_LOGGER_PRIORITY            = 1, // Low priority SD logging task
  TASK_BLE_OTA_PRIORITY              = 2, // Medium priority OTA task
  TASK_FSM_PRIORITY                  = 3, // High-medium priority system state coordination
  TASK_STATUS_LED_PRIORITY           = 1, // Low priority UI LED status rendering
  TASK_CALIB_GYRO_PRIORITY           = 1, // Low priority singular gyro calib worker
  TASK_CALIB_ACCEL_PRIORITY          = 1, // Low priority iterative accel calib worker
  TASK_CALIB_MAG_PRIORITY            = 1, // Low priority iterative mag calib worker
  TASK_CALIB_BARO_PRIORITY           = 1  // Low priority singular baro calib worker
};

enum TaskFrequenciesHz : uint32_t {
  TASK_ACCEL_GYRO_DAQ_FREQ_HZ       = 500,
  TASK_ACCEL_GYRO_LPF_FREQ_HZ       = 500,
  TASK_BARO_DAQ_FREQ_HZ             = 50,
  TASK_MAG_DAQ_FREQ_HZ              = 100,
  TASK_MAG_LPF_FREQ_HZ              = 100,
  TASK_BLE_TELEM_FREQ_HZ            = 100, // 100 Hz Unified Telemetry Evaluation Cadence (10 ms period)
  TASK_SD_LOGGER_FREQ_HZ            = 100,
  TASK_BLE_OTA_FREQ_HZ              = 0,   // Event-driven
  TASK_FSM_FREQ_HZ                  = 100, // 100 Hz FSM Evaluation Loop (10 ms period)
  TASK_STATUS_LED_FREQ_HZ           = 0,   // Event-driven pattern worker
  TASK_CALIB_GYRO_FREQ_HZ           = 200, // 200 Hz singular sampling
  TASK_CALIB_ACCEL_FREQ_HZ          = 200, // 200 Hz interactive sampling
  TASK_CALIB_MAG_FREQ_HZ            = 100, // 100 Hz interactive sampling
  TASK_CALIB_BARO_FREQ_HZ           = 50   // 50 Hz singular sampling
};

enum TaskSizesBytes : uint32_t {
  TASK_ACCEL_GYRO_DAQ_STACK_SIZE       = 4096,
  TASK_ACCEL_GYRO_LPF_STACK_SIZE       = 4096,
  TASK_BARO_DAQ_STACK_SIZE             = 4096,
  TASK_MAG_DAQ_STACK_SIZE              = 4096,
  TASK_MAG_LPF_STACK_SIZE              = 4096,
  TASK_BLE_TELEM_STACK_SIZE            = 4096,
  TASK_SD_LOGGER_STACK_SIZE            = 8192,
  TASK_BLE_OTA_STACK_SIZE              = 8192,
  TASK_FSM_STACK_SIZE                  = 4096,
  TASK_STATUS_LED_STACK_SIZE           = 2048,
  TASK_CALIB_GYRO_STACK_SIZE           = 4096,
  TASK_CALIB_ACCEL_STACK_SIZE          = 8192,
  TASK_CALIB_MAG_STACK_SIZE            = 8192,
  TASK_CALIB_BARO_STACK_SIZE           = 4096
};


// =============================================================================
// MATH, CONTROL AND DSP PARAMETERS
// =============================================================================
constexpr float GYRO_LPF_CUTOFF_FREQ_HZ  = 25.0f;
constexpr float ACCEL_LPF_CUTOFF_FREQ_HZ = 30.0f;
constexpr float MAG_LPF_CUTOFF_FREQ_HZ   = 10.0f;
constexpr uint8_t BMP180_DEFAULT_OSS     = 2; // BMP_OSS_HIGH_RES (13.5 ms conversion, 50 Hz compatible)

// Passive Sensor Calibration & Preflight Check Tolerances
constexpr float GYRO_PASSIVE_TOLERANCE_DPS    = 2.0f;  // +- 2 deg/s maximum stationary bias
constexpr float ACCEL_PASSIVE_TOLERANCE_MSS   = 0.5f;  // +- 0.5 m/s^2 deviation from standard 1g (9.81 m/s^2)
constexpr float MAG_PASSIVE_TOLERANCE_NORM    = 0.20f; // +- 20% deviation from expected normalized field
constexpr float STATIONARY_GYRO_VAR_DPS       = 0.8f;  // Variance threshold for stationary gyro (deg/s)
constexpr float STATIONARY_ACCEL_VAR_MSS      = 0.15f; // Variance threshold for stationary accel (m/s^2)
constexpr uint32_t STATIONARY_TIME_REQUIRED_MS = 1500; // Continuous stationary duration before passive check (1.5s)


// =============================================================================
// OTHER SETTINGS
// =============================================================================
constexpr uint8_t RGB_LED_DEFAULT_BRIGHTNESS = 64;
constexpr uint32_t USER_BTN_HOLD_DURATION_MS = 3000;
constexpr uint8_t LED_COLOR_SD_STOP_R        = 0;
constexpr uint8_t LED_COLOR_SD_STOP_G        = 255;
constexpr uint8_t LED_COLOR_SD_STOP_B        = 255;

// Active Calibration & Status LED Configuration
constexpr size_t   LED_CMD_QUEUE_DEPTH           = 16;
constexpr size_t   CALIB_REPORT_QUEUE_DEPTH      = 16;
constexpr bool     AUTO_REBOOT_AFTER_CALIBRATION = false;
constexpr uint32_t CALIB_MENU_DWELL_MS           = 1500;
constexpr uint32_t CALIB_SETTLING_DELAY_MS       = 2000; // Hands-off settling delay before sampling (2.0s)

// Magnetometer 3D Ellipsoid Calibration SIMD Parameters
constexpr uint16_t CALIB_MAG_SAMPLE_COUNT        = 1000;  // Point cloud buffer size in SRAM
constexpr uint16_t CALIB_MAG_MIN_SAMPLES         = 50;    // Minimum samples required to run solver
constexpr float    CALIB_MAG_MIN_DIST_GAUSS      = 0.025f; // Spatial thinning distance threshold (Gauss)
constexpr uint8_t  CALIB_MAG_EPOCHS_PER_CYCLE    = 2;     // Adam epochs computed per 10 ms RTOS cycle (1..5)
constexpr uint16_t CALIB_MAG_MAX_EPOCHS          = 100;   // Maximum Adam optimization epochs
constexpr float    CALIB_MAG_ADAM_LR             = 0.02f; // Adam learning rate
constexpr float    CALIB_MAG_ADAM_TOL            = 1e-6f; // Early stopping tolerance |Delta RMSE|


