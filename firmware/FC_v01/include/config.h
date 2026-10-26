#pragma once
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// =============================================================================
// MATHEMATICAL CONSTANTS & UNIT CONVERSIONS
// =============================================================================
#ifndef MATH_PI
constexpr float MATH_PI    = 3.14159265358979323846f;
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (0.017453292519943295f) // Multiplier to convert degrees to radians (pi / 180)
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG (57.295779513082320876f) // Multiplier to convert radians to degrees (180 / pi)
#endif

// =============================================================================
// SYSTEM & ARCHITECTURE FEATURE FLAGS
// =============================================================================
constexpr bool ENABLE_SIMD_ACCELERATION = true;  // Hardware SIMD Vector Math Acceleration Flag
#define DISABLE_I2C1_FOR_JTAG 0                  // Set to 1 if GPIO 19/20 are dedicated to hardware JTAG debugging

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

// ESC Motor PWM Output Pins
constexpr int ESC_PWM1_PIN      = 4;
constexpr int ESC_PWM2_PIN      = 3;
constexpr int ESC_PWM3_PIN      = 40;
constexpr int ESC_PWM4_PIN      = 1;

// ESC Motor PWM Hardware Settings (250 Hz Standard PWM, 1000-2000 us Pulse Width = 25-50% Duty Cycle)
constexpr uint8_t  MOTOR_COUNT                 = 4;
constexpr uint32_t ESC_PWM_FREQ_HZ             = 250;  // 250 Hz ESC carrier frequency (4,000 us period)
constexpr uint8_t  ESC_LEDC_TIMER_BIT_DEPTH    = 14;   // 14-bit timer resolution (16384 ticks max on ESP32-S3)
constexpr uint16_t ESC_PWM_DISARMED_US         = 1000; // Disarmed / zero-throttle pulse width (us) -> 25% duty
constexpr uint16_t ESC_PWM_MIN_US              = 1000; // Minimum pulse width corresponding to 0.0 normalized input (us)
constexpr uint16_t ESC_PWM_IDLE_US             = 1080; // Armed idle breakout threshold from bench test (us)
constexpr uint16_t ESC_PWM_MAX_US              = 2000; // Maximum pulse width corresponding to 1.0 normalized input (us) -> 50% duty
constexpr size_t   MOTOR_CMD_QUEUE_SIZE         = 4;    // Command queue depth for state arm/disarm triggers

// SPI Bus Pins (SD Card, LoRa & Peripherals)
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

// Peripherals & User Interface Pins
constexpr int USER_BTN_PIN      = 38;
constexpr int LORA_RST_PIN      = 41;
constexpr int SD_CARD_DET_PIN   = 18;
constexpr int IMU_MAG_DRDY_PIN  = 42;
constexpr int RGB_LED_PIN       = 2;  // WS2812 Single RGB Status LED on PCB v02 (GPIO2)

// =============================================================================
// BUS & PERIPHERAL HARDWARE OPERATING CONFIGURATIONS
// =============================================================================
// I2C Bus Operating Modes & Frequencies
constexpr bool     SYSTEM_I2C0_MODE_ASYNC       = false;
constexpr bool     SYSTEM_I2C1_MODE_ASYNC       = false;
constexpr uint32_t I2C_FREQ_HZ                  = 400000;

// SPI Bus Frequencies
constexpr uint32_t SPI_BUS_DEFAULT_FREQ_HZ     = 10000000; // 10 MHz Default SPI Bus
constexpr uint32_t SD_SPI_FREQ_HZ              = 20000000; // 20 MHz High-Speed SD Card SPI Bus

// INA226 Power Module Hardware & Calibration (0.5 mOhm Shunt on I2C1 Bus)
constexpr uint8_t  INA226_I2C_ADDR             = 0x40;
constexpr float    INA226_SHUNT_RESISTOR_OHM   = 0.0005f; // 0.5 mOhm (0.0005 Ohm)
constexpr float    INA226_CURRENT_LSB_MA       = 5.0f;    // 5 mA per LSB (millis scale)
constexpr float    INA226_POWER_LSB_MW         = 125.0f;  // 125 mW per LSB (millis scale)
constexpr float    INA226_VOLTAGE_LSB_MV       = 1.25f;   // 1.25 mV per LSB (millis scale)
constexpr uint16_t INA226_CALIBRATION_VAL      = 2048;    // 0x0800 (exact integer for 0.5mOhm & 5mA LSB)

// BMP180 Barometer Hardware Settings
constexpr uint8_t  BMP180_DEFAULT_OSS          = 2;       // BMP_OSS_HIGH_RES (13.5 ms conversion, 50 Hz compatible)

// =============================================================================
// OPERATING SYSTEM ABSTRACTION LAYER (OSAL) & IPC SYNCHRONIZATION
// =============================================================================
constexpr int      RING_BUF_MAX_SIZE           = 100;     // Maximum allowable ring buffer size
constexpr int      RING_BUF_SIZE               = 10;      // Unified inter-task ring buffer capacity
constexpr uint32_t BUS_MUTEX_TIMEOUT_MS        = 20;      // Global xSemaphoreTake timeout (20 ms, must exceed Wire timeout + max I2C transaction)

// =============================================================================
// CENTRALIZED FREERTOS TASK CONFIGURATIONS
// =============================================================================

// Task Names
constexpr const char* TASK_ACCEL_GYRO_DAQ_NAME        = "ACCEL_GYRO_DAQ";
constexpr const char* TASK_ACCEL_GYRO_PREPROCESS_NAME = "ACCEL_GYRO_PREPROCESS";
constexpr const char* TASK_BARO_DAQ_NAME              = "BARO_DAQ";
constexpr const char* TASK_MAG_DAQ_NAME               = "MAG_DAQ";
constexpr const char* TASK_MAG_PREPROCESS_NAME        = "MAG_PREPROCESS";
constexpr const char* TASK_BLE_TELEM_NAME             = "BLE_TELEM";
constexpr const char* TASK_SD_LOGGER_NAME             = "SD_LOGGER";
constexpr const char* TASK_BLE_OTA_NAME               = "BLE_OTA";
constexpr const char* TASK_FSM_NAME                   = "FSM_MAIN";
constexpr const char* TASK_STATUS_LED_NAME            = "STATUS_LED";
constexpr const char* TASK_CALIB_GYRO_NAME            = "CALIB_GYRO";
constexpr const char* TASK_CALIB_ACCEL_NAME           = "CALIB_ACCEL";
constexpr const char* TASK_CALIB_MAG_NAME             = "CALIB_MAG";
constexpr const char* TASK_CALIB_BARO_NAME            = "CALIB_BARO";
constexpr const char* TASK_POWER_DAQ_NAME             = "POWER_DAQ";
constexpr const char* TASK_RC_RECEIVER_NAME          = "RC_RECEIVER";
constexpr const char* TASK_EKF_ROLL_PITCH_NAME       = "EKF_ROLL_PITCH";
constexpr const char* TASK_EKF_YAW_NAME              = "EKF_YAW";
constexpr const char* TASK_EKF_ALTITUDE_NAME         = "EKF_ALTITUDE";
constexpr const char* TASK_MOTOR_NAME                = "MOTOR";

// Task Unique IDs
enum TaskIDs : uint32_t {
  TASK_ACCEL_GYRO_DAQ_ID = 0,
  TASK_ACCEL_GYRO_PREPROCESS_ID,
  TASK_BARO_DAQ_ID,
  TASK_MAG_DAQ_ID,
  TASK_MAG_PREPROCESS_ID,
  TASK_BLE_TELEM_ID,
  TASK_SD_LOGGER_ID,
  TASK_BLE_OTA_ID,
  TASK_FSM_ID,
  TASK_STATUS_LED_ID,
  TASK_CALIB_GYRO_ID,
  TASK_CALIB_ACCEL_ID,
  TASK_CALIB_MAG_ID,
  TASK_CALIB_BARO_ID,
  TASK_POWER_DAQ_ID,
  TASK_RC_RECEIVER_ID,
  TASK_EKF_ROLL_PITCH_ID,
  TASK_EKF_YAW_ID,
  TASK_EKF_ALTITUDE_ID,
  TASK_MOTOR_ID,
  TASK_COUNT
};

// Task Core Affinity
enum TaskCores : int32_t {
  TASK_ACCEL_GYRO_DAQ_CORE        = tskNO_AFFINITY,
  TASK_ACCEL_GYRO_PREPROCESS_CORE = tskNO_AFFINITY,
  TASK_BARO_DAQ_CORE              = tskNO_AFFINITY,
  TASK_MAG_DAQ_CORE               = tskNO_AFFINITY,
  TASK_MAG_PREPROCESS_CORE        = tskNO_AFFINITY,
  TASK_BLE_TELEM_CORE             = tskNO_AFFINITY,
  TASK_SD_LOGGER_CORE             = tskNO_AFFINITY,
  TASK_BLE_OTA_CORE               = tskNO_AFFINITY,
  TASK_FSM_CORE                   = tskNO_AFFINITY,
  TASK_STATUS_LED_CORE            = tskNO_AFFINITY,
  TASK_CALIB_GYRO_CORE            = tskNO_AFFINITY,
  TASK_CALIB_ACCEL_CORE           = tskNO_AFFINITY,
  TASK_CALIB_MAG_CORE             = tskNO_AFFINITY,
  TASK_CALIB_BARO_CORE            = tskNO_AFFINITY,
  TASK_POWER_DAQ_CORE             = tskNO_AFFINITY,
  TASK_RC_RECEIVER_CORE           = tskNO_AFFINITY,
  TASK_EKF_ROLL_PITCH_CORE        = tskNO_AFFINITY,
  TASK_EKF_YAW_CORE               = tskNO_AFFINITY,
  TASK_EKF_ALTITUDE_CORE          = tskNO_AFFINITY,
  TASK_MOTOR_CORE                 = tskNO_AFFINITY
};

// Task Real-Time Priorities (System 4 Priority Levels)
enum TaskPriorities : UBaseType_t {
  TASK_ACCEL_GYRO_DAQ_PRIORITY        = 3, // 500 Hz Realtime DAQ Loop
  TASK_ACCEL_GYRO_PREPROCESS_PRIORITY = 3, // 500 Hz Realtime Preprocess Loop
  TASK_BARO_DAQ_PRIORITY              = 2, // Medium priority sensor DAQ
  TASK_MAG_DAQ_PRIORITY               = 2, // Medium priority sensor DAQ
  TASK_MAG_PREPROCESS_PRIORITY        = 2, // Medium priority sensor Preprocess
  TASK_BLE_TELEM_PRIORITY             = 1, // Low priority unified telemetry dispatcher task
  TASK_SD_LOGGER_PRIORITY             = 1, // Low priority SD logging task
  TASK_BLE_OTA_PRIORITY               = 2, // Medium priority OTA task
  TASK_FSM_PRIORITY                   = 3, // High-medium priority system state coordination
  TASK_STATUS_LED_PRIORITY            = 1, // Low priority UI LED status rendering
  TASK_CALIB_GYRO_PRIORITY            = 1, // Low priority singular gyro calib worker
  TASK_CALIB_ACCEL_PRIORITY           = 1, // Low priority iterative accel calib worker
  TASK_CALIB_MAG_PRIORITY             = 1, // Low priority iterative mag calib worker
  TASK_CALIB_BARO_PRIORITY            = 1, // Low priority singular baro calib worker
  TASK_POWER_DAQ_PRIORITY             = 2, // Medium priority sensor DAQ (50 Hz)
  TASK_RC_RECEIVER_PRIORITY           = 2, // Medium priority RC PPM receiver DAQ (50 Hz)
  TASK_EKF_ROLL_PITCH_PRIORITY        = 3, // Realtime pilot attitude loop (500 Hz)
  TASK_EKF_YAW_PRIORITY               = 2, // Medium priority navigator loop (100 Hz)
  TASK_EKF_ALTITUDE_PRIORITY          = 2, // Medium priority vertical velocity & altitude loop (100 Hz)
  TASK_MOTOR_PRIORITY                 = 4  // Realtime motor mixer loop (250 Hz)
};

// Task Nominal Execution Frequencies
enum TaskFrequenciesHz : uint32_t {
  TASK_ACCEL_GYRO_DAQ_FREQ_HZ        = 500,
  TASK_ACCEL_GYRO_PREPROCESS_FREQ_HZ = 500,
  TASK_BARO_DAQ_FREQ_HZ              = 50,
  TASK_MAG_DAQ_FREQ_HZ               = 100,
  TASK_MAG_PREPROCESS_FREQ_HZ        = 100,
  TASK_BLE_TELEM_FREQ_HZ             = 100,
  TASK_SD_LOGGER_FREQ_HZ             = 100,
  TASK_BLE_OTA_FREQ_HZ               = 0,   // Event-driven
  TASK_FSM_FREQ_HZ                   = 100,
  TASK_STATUS_LED_FREQ_HZ            = 0,   // Event-driven pattern worker
  TASK_CALIB_GYRO_FREQ_HZ            = 200, // 200 Hz singular sampling
  TASK_CALIB_ACCEL_FREQ_HZ           = 200, // 200 Hz interactive sampling
  TASK_CALIB_MAG_FREQ_HZ             = 100, // 100 Hz interactive sampling
  TASK_CALIB_BARO_FREQ_HZ            = 50,  // 50 Hz singular sampling
  TASK_POWER_DAQ_FREQ_HZ             = 50,
  TASK_RC_RECEIVER_FREQ_HZ           = 50,
  TASK_EKF_ROLL_PITCH_FREQ_HZ        = 500,
  TASK_EKF_YAW_FREQ_HZ               = 100,
  TASK_EKF_ALTITUDE_FREQ_HZ          = 100,
  TASK_MOTOR_FREQ_HZ                 = 250
};

// Task Stack Sizes in Bytes
enum TaskSizesBytes : uint32_t {
  TASK_ACCEL_GYRO_DAQ_STACK_SIZE        = 4096,
  TASK_ACCEL_GYRO_PREPROCESS_STACK_SIZE = 4096,
  TASK_BARO_DAQ_STACK_SIZE              = 4096,
  TASK_MAG_DAQ_STACK_SIZE               = 4096,
  TASK_MAG_PREPROCESS_STACK_SIZE        = 4096,
  TASK_BLE_TELEM_STACK_SIZE             = 4096,
  TASK_SD_LOGGER_STACK_SIZE             = 8192,
  TASK_BLE_OTA_STACK_SIZE               = 8192,
  TASK_FSM_STACK_SIZE                   = 4096,
  TASK_STATUS_LED_STACK_SIZE            = 2048,
  TASK_CALIB_GYRO_STACK_SIZE            = 4096,
  TASK_CALIB_ACCEL_STACK_SIZE           = 12288,
  TASK_CALIB_MAG_STACK_SIZE             = 20480,
  TASK_CALIB_BARO_STACK_SIZE            = 4096,
  TASK_POWER_DAQ_STACK_SIZE             = 4096,
  TASK_RC_RECEIVER_STACK_SIZE           = 4096,
  TASK_EKF_ROLL_PITCH_STACK_SIZE        = 8192,
  TASK_EKF_YAW_STACK_SIZE               = 4096,
  TASK_EKF_ALTITUDE_STACK_SIZE          = 4096,
  TASK_MOTOR_STACK_SIZE                 = 4096
};

// =============================================================================
// EXTENDED KALMAN FILTER (EKF) ATTITUDE ESTIMATION CONFIGURATION
// =============================================================================
// Geomagnetic Dip Angle (Inclination) for target destination: Ho Chi Minh City, Vietnam (WMM: 10.75° N, 106.67° E)
constexpr float MAGNETIC_DIP_DEG       = 9.78f;                        // Dip angle in degrees (+9.78 deg N in Ho Chi Minh City, Vietnam)
constexpr float MAGNETIC_DIP           = MAGNETIC_DIP_DEG * DEG_TO_RAD; // Dip angle in radians

// Unit Reference Magnetic Vector b_earth = [bx, 0, bz]^T (Strictly on Unit Sphere, norm = 1.0)
// Mathematical Derivation:
// In the NED (North-East-Down) coordinate frame:
//   bx = cos(MAGNETIC_DIP) -> horizontal component pointing to magnetic North
//   by = 0.0               -> zero declination alignment in local magnetic frame
//   bz = sin(MAGNETIC_DIP) -> vertical downward component pointing into the Earth
//   ||b_earth||^2 = bx^2 + bz^2 = cos^2(MAGNETIC_DIP) + sin^2(MAGNETIC_DIP) = 1.0
constexpr float EKF_EARTH_BX           = __builtin_cosf(MAGNETIC_DIP); // cos(MAGNETIC_DIP) ≈ 0.98546f
constexpr float EKF_EARTH_BZ           = __builtin_sinf(MAGNETIC_DIP); // sin(MAGNETIC_DIP) ≈ 0.16985f

// Empirical sensor noise variances derived from log file
constexpr float EKF_DEFAULT_Q_QUAT_VAR = 0.0001f;
constexpr float EKF_DEFAULT_Q_BIAS_VAR = 1e-6f;
constexpr float EKF_DEFAULT_R_ACCEL_X  = 2.25e-6f; // (0.0015 g)^2
constexpr float EKF_DEFAULT_R_ACCEL_Y  = 1.69e-6f; // (0.0013 g)^2
constexpr float EKF_DEFAULT_R_ACCEL_Z  = 4.84e-6f; // (0.0022 g)^2
constexpr float EKF_DEFAULT_R_ACCEL    = 0.05f;    // Baseline flight noise with vibration margin
constexpr float EKF_DEFAULT_R_MAG_X    = 6.76e-6f; // (0.0026 Gauss)^2
constexpr float EKF_DEFAULT_R_MAG_Y    = 4.84e-6f; // (0.0022 Gauss)^2
constexpr float EKF_DEFAULT_R_MAG_Z    = 4.49e-5f; // (0.0067 Gauss)^2
constexpr float EKF_DEFAULT_R_MAG      = 0.2f;     // Environmental magnetic noise margin

// =============================================================================
// EXTENDED KALMAN FILTER (EKF) VERTICAL VELOCITY & ALTITUDE ESTIMATION CONFIGURATION
// =============================================================================
constexpr float STANDARD_GRAVITY_MSS            = 9.80665f;  // Standard acceleration of gravity g (m/s^2)
constexpr float BARO_SEA_LEVEL_PRESSURE_PA      = 101325.0f; // Standard atmospheric sea-level pressure P0 fallback (Pa)
constexpr float EKF_ALT_DEFAULT_Q_POS_VAR       = 1e-4f;     // Altitude process noise variance (m^2)
constexpr float EKF_ALT_DEFAULT_Q_VEL_VAR       = 1e-3f;     // Vertical velocity process noise variance ((m/s)^2)
constexpr float EKF_ALT_DEFAULT_Q_BIAS_VAR      = 1e-6f;     // Vertical accel bias random walk variance ((m/s^2)^2)
constexpr float EKF_ALT_DEFAULT_R_BARO_PA_VAR   = 17.8263f;  // Empirical BMP180 OSS=2 variance from LOG_0032 (4.2221 Pa)^2
constexpr float EKF_ALT_DEFAULT_R_BARO_MARGIN   = 7.1737f;   // Propwash & aerodynamic pressure noise margin (Pa^2)

// =============================================================================
// BLE CONFIGURATION, GATT SERVICE DEFINITIONS & TELEMETRY BITMASK
// =============================================================================
constexpr const char* BLE_DEVICE_NAME             = "FC_v02";
constexpr uint16_t    BLE_MTU_SIZE                = 512; // bytes
constexpr uint32_t    BLE_TELEM_MIN_PACKET_FREQ_HZ = 10;

// Telemetry Samples Per Packet
constexpr uint8_t     BLE_GYRO_SAMPLES_PER_PACKET      = 31; // 31 samples x 16B = 496B payload
constexpr uint8_t     BLE_ACCEL_SAMPLES_PER_PACKET     = 31; // 31 samples x 16B = 496B payload
constexpr uint8_t     BLE_MAG_SAMPLES_PER_PACKET       = 31; // 31 samples x 16B = 496B payload
constexpr uint8_t     BLE_BARO_SAMPLES_PER_PACKET      = 63; // 63 samples x 8B = 504B payload
constexpr uint8_t     BLE_ATT_QUAT_SAMPLES_PER_PACKET  = 25; // 25 samples x 20B = 500B payload
constexpr uint8_t     BLE_ATT_EULER_SAMPLES_PER_PACKET = 31; // 31 samples x 16B = 496B payload
constexpr uint8_t     BLE_POWER_SAMPLES_PER_PACKET     = 25; // 25 samples x 20B = 500B payload
constexpr uint8_t     BLE_RC_SAMPLES_PER_PACKET        = 17; // 17 samples x 28B = 476B payload

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
constexpr const char* BLE_CHAR_POWER_UUID           = "3e5a7b1c-9d2f-4e8a-0b3c-6f1a8d2e4c5b";
constexpr const char* BLE_CHAR_RC_CHANNELS_UUID     = "5b8c1a2e-4f9a-4c72-9e3d-8b1a2c3d4e5f";

// BLE OTA Service (Dual-bank firmware update support)
constexpr const char* BLE_SERVICE_OTA_UUID          = "23408888-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr const char* BLE_CHAR_OTA_CONTROL_UUID     = "23408889-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr const char* BLE_CHAR_OTA_DATA_UUID        = "2340888a-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr uint32_t    OTA_BOOT_WINDOW_TIMEOUT_MS    = 10000; // Boot listening window: 10000 ms (10s)
constexpr size_t      OTA_DATA_QUEUE_DEPTH          = 32;    // Pipelined OTA queue depth (~16.5 KB SRAM, reclaimed after boot window)

// BLE Telemetry Bitmask & Single Source-of-Truth
enum BleTelemBitmask : uint16_t {
  BLE_TELEM_MASK_NONE        = 0,
  BLE_TELEM_MASK_GYRO_RAW    = (1 << 0),
  BLE_TELEM_MASK_GYRO_FILT   = (1 << 1),
  BLE_TELEM_MASK_ACCEL_RAW   = (1 << 2),
  BLE_TELEM_MASK_ACCEL_FILT  = (1 << 3),
  BLE_TELEM_MASK_MAG_RAW     = (1 << 4),
  BLE_TELEM_MASK_MAG_FILT    = (1 << 5),
  BLE_TELEM_MASK_BARO_RAW    = (1 << 6),
  BLE_TELEM_MASK_BARO_FILT   = (1 << 7),
  BLE_TELEM_MASK_ATT_QUAT    = (1 << 8),
  BLE_TELEM_MASK_ATT_EULER   = (1 << 9),
  BLE_TELEM_MASK_POWER       = (1 << 10),
  BLE_TELEM_MASK_RC_CHANNELS = (1 << 11)
};

// Multi-line commentable/uncommentable BLE telemetry mask interface
constexpr uint16_t ACTIVE_BLE_TELEM_MASK =
    BLE_TELEM_MASK_NONE
    | BLE_TELEM_MASK_GYRO_RAW
    | BLE_TELEM_MASK_ACCEL_RAW
    | BLE_TELEM_MASK_GYRO_FILT
    | BLE_TELEM_MASK_ACCEL_FILT
    | BLE_TELEM_MASK_MAG_RAW
    | BLE_TELEM_MASK_MAG_FILT
    // | BLE_TELEM_MASK_BARO_RAW
    // | BLE_TELEM_MASK_BARO_FILT
    // | BLE_TELEM_MASK_ATT_QUAT
    | BLE_TELEM_MASK_ATT_EULER
    // | BLE_TELEM_MASK_POWER
    | BLE_TELEM_MASK_RC_CHANNELS
    ;

// =============================================================================
// BLACKBOX SD CARD & SPI LOGGING CONFIGURATION
// =============================================================================
constexpr const char* SD_MOUNT_POINT                = "/sd";
constexpr const char* BOARD_IDENTIFIER              = "esp32-s3-fc-v02-n8r8";

enum LogPacketType : uint8_t {
  PKT_TYPE_IMU_RAW   = 0x01,
  PKT_TYPE_IMU_FILT  = 0x02,
  PKT_TYPE_MAG_RAW   = 0x03,
  PKT_TYPE_MAG_FILT  = 0x04,
  PKT_TYPE_BARO_RAW  = 0x05,
  PKT_TYPE_BARO_FILT = 0x06,
  PKT_TYPE_ATT_QUAT  = 0x07,
  PKT_TYPE_ATT_EULER = 0x08,
  PKT_TYPE_POWER     = 0x09,
  PKT_TYPE_RC        = 0x0A
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
  LOG_MASK_ATT_EULER = (1 << 7),
  LOG_MASK_POWER     = (1 << 8),
  LOG_MASK_RC        = (1 << 9)
};

// Multi-line commentable/uncommentable SD logging mask interface
constexpr uint16_t ACTIVE_SD_LOG_PACKETS_MASK =
    LOG_MASK_NONE
    | LOG_MASK_IMU_RAW
    | LOG_MASK_IMU_FILT
    | LOG_MASK_MAG_RAW
    | LOG_MASK_MAG_FILT
    | LOG_MASK_BARO_RAW
    | LOG_MASK_BARO_FILT
    // | LOG_MASK_ATT_QUAT
    // | LOG_MASK_ATT_EULER
    // | LOG_MASK_POWER
    // | LOG_MASK_RC
    ;

enum LogLimitMode : uint8_t {
  LOG_LIMIT_STOP_ON_SIZE       = 0,
  LOG_LIMIT_CIRCULAR_OVERWRITE = 1
};

constexpr LogLimitMode LOG_LIMIT_POLICY             = LOG_LIMIT_STOP_ON_SIZE;
constexpr uint32_t     LOG_MAX_FILE_SIZE_BYTES      = 50 * 1024 * 1024;
constexpr uint32_t     LOG_SECTOR_TIMEOUT_MS        = 100;
constexpr uint32_t     LOG_METADATA_SYNC_PERIOD_MS  = 1000;

constexpr uint8_t      LED_COLOR_SD_STOP_R          = 0;
constexpr uint8_t      LED_COLOR_SD_STOP_G          = 255;
constexpr uint8_t      LED_COLOR_SD_STOP_B          = 255;

// =============================================================================
// MATH, DIGITAL SIGNAL PROCESSING (DSP) & CONTROL PARAMETERS
// =============================================================================
// Digital Low-Pass Filters Cutoff Parameters
constexpr float GYRO_LPF_CUTOFF_WN_RAD_S            = 50.0f;  // Natural angular frequency wn (rad/s)
constexpr float GYRO_LPF_ZETA                       = 0.55f;  // Damping ratio zeta (dimensionless)
constexpr float GYRO_LPF_CUTOFF_FREQ_HZ             = GYRO_LPF_CUTOFF_WN_RAD_S / (2.0f * MATH_PI); // Cutoff frequency (~7.9577 Hz)
constexpr float ACCEL_LPF_CUTOFF_FREQ_HZ            = 30.0f;  // Cutoff frequency (Hz)
constexpr float MAG_LPF_CUTOFF_FREQ_HZ              = 10.0f;  // Cutoff frequency (Hz)

// =============================================================================
// PRE-FLIGHT SAFETY, HEALTH CHECKS & ARMING GATES
// =============================================================================
// Stationary Condition & Passive Calibration Tolerances
constexpr float    GYRO_PASSIVE_TOLERANCE_DPS       = 2.0f;  // +- 2 deg/s maximum stationary bias
constexpr float    ACCEL_PASSIVE_TOLERANCE_MSS      = 0.5f;  // +- 0.5 m/s^2 deviation from standard 1g (9.81 m/s^2)
constexpr float    MAG_PASSIVE_TOLERANCE_NORM       = 0.20f; // +- 20% deviation from expected normalized field
constexpr float    STATIONARY_GYRO_VAR_DPS          = 0.8f;  // Variance threshold for stationary gyro (deg/s)
constexpr float    STATIONARY_ACCEL_VAR_MSS         = 0.15f; // Variance threshold for stationary accel (m/s^2)
constexpr uint32_t STATIONARY_TIME_REQUIRED_MS      = 1500;  // Continuous stationary duration before passive check (1.5s)
constexpr uint32_t PREFLIGHT_STATIONARY_TIMEOUT_MS   = 5000;  // 5.0s settling window before stationary timeout
constexpr float    PREFLIGHT_MAX_TILT_DEG            = 10.0f; // Maximum platform incline angle before arming (10 degrees)

// Takeoff & In-Flight Transition Safety Thresholds
constexpr uint32_t TAKEOFF_VERIFY_DURATION_MS       = 5000;   // 5.0 seconds checking window
constexpr uint32_t READY_GROUND_IDLE_TIMEOUT_MS     = 2000;   // 2.0 seconds ground idle pacing
constexpr float    TAKEOFF_MAX_TILT_DEG             = 30.0f;  // Maximum allowed roll/pitch angle during takeoff (deg)
constexpr float    TAKEOFF_MAX_ANGULAR_RATE_DPS     = 150.0f; // Maximum allowed angular rate before runaway flip guard trips (deg/s)
constexpr float    TAKEOFF_MAX_YAW_DIVERGENCE_DEG   = 30.0f;  // Maximum yaw deviation from startup heading (deg)
constexpr uint16_t RC_THROTTLE_MIN_US               = 1000;   // Minimum throttle PWM (us)
constexpr uint16_t RC_THROTTLE_MAX_US               = 2000;   // Maximum throttle PWM (us)
constexpr uint16_t RC_THROTTLE_IDLE_US              = 1150;   // Ground idle throttle threshold (us)
constexpr uint16_t RC_THROTTLE_LAUNCH_MIN_US        = 1400;   // 40% launch window lower bound (us)
constexpr uint16_t RC_THROTTLE_LAUNCH_MAX_US        = 1800;   // 80% launch window upper bound (us)
constexpr uint16_t RC_STICK_CENTER_DEADBAND_US      = 50;     // +-5% centering deadband (1450 - 1550 us)
constexpr uint16_t RC_ARM_SWITCH_THRESHOLD_US       = 1500;   // Ch5 Aux1 arm switch threshold (>1500 us = ARMED)

// Preflight Sensor Data Stream Health Check Latency & Ranges
constexpr uint32_t SENSOR_STREAM_IMU_TIMEOUT_US     = 50000;   // 50 ms maximum latency for 500 Hz IMU (accel/gyro)
constexpr uint32_t SENSOR_STREAM_MAG_TIMEOUT_US     = 100000;  // 100 ms maximum latency for 100 Hz Magnetometer
constexpr uint32_t SENSOR_STREAM_BARO_TIMEOUT_US    = 200000;  // 200 ms maximum latency for 50 Hz Barometer
constexpr uint32_t SENSOR_STREAM_POWER_TIMEOUT_US   = 200000;  // 200 ms maximum latency for 50 Hz Power DAQ
constexpr uint32_t SENSOR_STREAM_HEALTHY_TIME_MS    = 500;     // Continuous healthy streaming required (500 ms)
constexpr uint32_t SENSOR_STREAM_STARTUP_TIMEOUT_MS = 2500;    // Maximum time allowed for all sensor streams to be ready (2.5 s)
constexpr float    SENSOR_STREAM_BARO_MIN_PA        = 30000.0f;  // Minimum valid atmospheric pressure (300 hPa)
constexpr float    SENSOR_STREAM_BARO_MAX_PA        = 120000.0f; // Maximum valid atmospheric pressure (1200 hPa)

// Preflight Battery Health & Arming Voltage Thresholds (Millis scale)
constexpr float    BATTERY_CELL_MIN_VOLTAGE_MV      = 3300.0f; // 3.3V per cell minimum arming threshold
constexpr float    BATTERY_CELL_WARN_VOLTAGE_MV     = 3500.0f; // 3.5V per cell warning threshold
constexpr float    BATTERY_CELL_MAX_VOLTAGE_MV      = 4200.0f; // 4.20V per cell maximum (LiHV)

// =============================================================================
// INTERACTIVE ACTIVE SENSOR CALIBRATION PARAMETERS
// =============================================================================
// Active Calibration Menu & Coordination
constexpr size_t   CALIB_REPORT_QUEUE_DEPTH         = 16;
constexpr bool     AUTO_REBOOT_AFTER_CALIBRATION    = false;
constexpr uint32_t CALIB_MENU_DWELL_MS              = 1500;
constexpr uint32_t CALIB_SETTLING_DELAY_MS          = 2000; // Hands-off settling delay before sampling (2.0s)

// Gyroscope Stationary Calibration
constexpr uint16_t CALIB_GYRO_SAMPLE_COUNT          = 400;  // 2.0s stationary sampling at 200 Hz

// Barometer Ground Reference Baseline
constexpr uint16_t CALIB_BARO_SAMPLE_COUNT          = 150;  // 3.0s ground pressure sampling at 50 Hz

// Accelerometer 6-Point 3D Ellipsoid SIMD Calibration
constexpr uint8_t  CALIB_ACCEL_ORIENTATIONS_COUNT       = 6;     // Total orientations required
constexpr uint8_t  CALIB_ACCEL_SAMPLES_PER_ORIENTATION  = 50;    // Samples collected per locked orientation
constexpr uint16_t CALIB_ACCEL_SAMPLE_COUNT             = 300;   // Total point cloud buffer capacity (6 * 50)
constexpr uint32_t CALIB_ACCEL_STATIONARY_HOLD_MS       = 2000;  // Continuous stationary duration required (2.0s)
constexpr float    CALIB_ACCEL_STATIONARY_TOL_G         = 0.03f;  // Maximum allowed axis spread in resting state (g)
constexpr float    CALIB_ACCEL_MOTION_UNLATCH_G         = 0.10f;  // Minimum motion delta required to unlock previous pose (g)
constexpr float    CALIB_ACCEL_MIN_DIVERGENCE_COS       = 0.7071f; // Maximum dot product cosine for 45 deg separation
constexpr uint8_t  CALIB_ACCEL_EPOCHS_PER_CYCLE         = 2;     // Adam epochs computed per 5 ms cycle (200 Hz)
constexpr uint16_t CALIB_ACCEL_MAX_EPOCHS               = 100;   // Maximum Adam optimization epochs
constexpr float    CALIB_ACCEL_ADAM_LR                  = 0.02f; // Adam learning rate
constexpr float    CALIB_ACCEL_ADAM_TOL                 = 1e-6f; // Early stopping tolerance |Delta RMSE|

// Magnetometer 3D Ellipsoid SIMD Calibration
constexpr uint16_t CALIB_MAG_SAMPLE_COUNT           = 1000;  // Point cloud buffer size in SRAM
constexpr uint16_t CALIB_MAG_MIN_SAMPLES            = 50;    // Minimum samples required to run solver
constexpr float    CALIB_MAG_MIN_DIST_GAUSS         = 0.025f; // Spatial thinning distance threshold (Gauss)
constexpr uint8_t  CALIB_MAG_EPOCHS_PER_CYCLE       = 2;     // Adam epochs computed per 10 ms RTOS cycle (1..5)
constexpr uint16_t CALIB_MAG_MAX_EPOCHS             = 100;   // Maximum Adam optimization epochs
constexpr float    CALIB_MAG_ADAM_LR                = 0.02f; // Adam learning rate
constexpr float    CALIB_MAG_ADAM_TOL               = 1e-6f; // Early stopping tolerance |Delta RMSE|

// =============================================================================
// USER INTERFACE & PERIPHERAL SETTINGS
// =============================================================================
constexpr uint8_t  RGB_LED_DEFAULT_BRIGHTNESS       = 64;
constexpr size_t   LED_CMD_QUEUE_DEPTH              = 16;
constexpr uint32_t USER_BTN_HOLD_DURATION_MS        = 3000;
