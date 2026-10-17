#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "middleware/ring_buffer.h"
#include "drivers/telemetry/ble_ota_packet.h"
#include "core/math/matrix.h"
#include "drivers/led/ws2812.h"
#include "drivers/rc_rx/fs_ia6b.h"

// =============================================================================
// LAYER 3 MIDDLEWARE
// =============================================================================

// System Mutex Handles (I2C Bus & Status LED Synchronization Locks)
extern SemaphoreHandle_t i2c0_mutex;
extern SemaphoreHandle_t i2c1_mutex;
extern SemaphoreHandle_t status_led_mutex;
extern SemaphoreHandle_t spi_bus_mutex;
extern SemaphoreHandle_t flash_mutex;
extern SemaphoreHandle_t ekf_state_mutex;
extern TaskHandle_t app_task_handles[TASK_COUNT];
extern QueueHandle_t ota_data_queue;
extern OtaStartPayload ota_start_payload;
extern volatile bool ota_abort_requested;
extern volatile bool sd_stop_logging_requested;

// RGB LED Subsystem Command Structures (FSM-exclusive Producer)
enum LedPattern : uint8_t {
  LED_PATTERN_OFF = 0,
  LED_PATTERN_SOLID,
  LED_PATTERN_BLINK_SLOW,
  LED_PATTERN_BLINK_FAST,
  LED_PATTERN_BREATHE,
  LED_PATTERN_FLASH_BURST
};

struct LedCommand {
  LedPattern     pattern;
  PaletteColorId color;
};

// Calibration Subsystem Reporting Structures (Worker Producer, FSM Consumer)
enum CalibTarget : uint8_t {
  CALIB_TARGET_NONE = 0,
  CALIB_TARGET_GYRO,
  CALIB_TARGET_ACCEL,
  CALIB_TARGET_MAG,
  CALIB_TARGET_BARO
};

enum CalibStatus : uint8_t {
  CALIB_STATUS_IDLE = 0,
  CALIB_STATUS_SETTLING,      // Pre-sampling hands-off settling phase
  CALIB_STATUS_SAMPLING,      // Active stationary data acquisition phase
  CALIB_STATUS_HEALTHCHECK,
  CALIB_STATUS_STEP_LOCKED,   // Interactive step confirmed
  CALIB_STATUS_COMPUTING,     // End-stage iterative solver running
  CALIB_STATUS_COMPLETED,     // Finished successfully, waiting for FSM deletion
  CALIB_STATUS_FAILED         // Failed/timed out, waiting for FSM deletion
};

struct CalibReportMsg {
  CalibTarget target;
  CalibStatus status;
  uint8_t     current_step;   // e.g. 0..5 for 6-point accel
  uint8_t     progress_pct;   // 0..100%
  uint32_t    timestamp_us;   // Healthcheck timestamp
};

extern QueueHandle_t led_cmd_queue;
extern QueueHandle_t calib_report_queue;

// Sensor Calibration Shared IPC State (Layer 3 Centralized State)
class FlashStorageDriver;

struct SensorCalibrationData {
  Vec3<float>         gyro_bias{0.0f, 0.0f, 0.0f};   // Gyroscope zero-rate bias offsets (deg/s)
  Vec3<float>         accel_bias{0.0f, 0.0f, 0.0f};  // Accelerometer zero-g bias offset v0 (g)
  Vec3<float>         accel_scale{1.0f, 1.0f, 1.0f}; // Accelerometer 3-axis sensitivity scaling s (g^-1)
  Vec3<float>         mag_bias{0.0f, 0.0f, 0.0f};    // Magnetometer hard-iron bias offset v0 (Gauss)
  Matrix<float, 3, 3> mag_soft_iron{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}; // Magnetometer soft-iron matrix W * B0
  float               mag_field_ref{1.0f};           // Geomagnetic reference field B0 (Gauss)
  bool                is_valid{false};
};

extern SensorCalibrationData calib_data;
extern FlashStorageDriver*   calib_storage;

// Hardware Button Event Flags (Decoupled Layer 3 IPC Primitive)
enum BtnEventMasks : uint8_t {
  BTN_EVT_NONE        = 0,
  BTN_EVT_SHORT_CLICK = (1 << 0),
  BTN_EVT_LONG_HOLD   = (1 << 1)
};

uint8_t ipc_pop_btn_events();
void IRAM_ATTR user_btn_isr();

struct OtaDataChunk {
  uint16_t length;
  uint8_t data[512];
};

// Thread-Safe Inter-Task Ring Buffers
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> accel_raw;
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> gyro_raw;
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> mag_raw;
extern RingBuffer<float, RING_BUF_SIZE>       pressure;

// Power & Battery Measurement Data Structure (Millis Scale)
struct PowerMeasurement {
  float voltage_mv; // Bus voltage in millivolts (mV)
  float current_ma; // Current in milliamps (mA)
  float power_mw;   // Power in milliwatts (mW)
  float energy_mah; // Accumulated energy in milliamp-hours (mAh)
};

extern RingBuffer<PowerMeasurement, RING_BUF_SIZE> power_data;

// Thread-Safe Timestamp & Filtered Inter-Task Ring Buffers
extern RingBuffer<uint32_t, RING_BUF_SIZE>    raw_accel_gyro_timestamp;
extern RingBuffer<uint32_t, RING_BUF_SIZE>    raw_mag_timestamp;
extern RingBuffer<uint32_t, RING_BUF_SIZE>    pressure_timestamp;
extern RingBuffer<uint32_t, RING_BUF_SIZE>    power_timestamp;

extern RingBuffer<Vec3<float>, RING_BUF_SIZE> accel;
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> gyro;
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> mag;

extern RingBuffer<uint32_t, RING_BUF_SIZE>    filtered_accel_gyro_timestamp;
extern RingBuffer<uint32_t, RING_BUF_SIZE>    filtered_mag_timestamp;

extern RingBuffer<RcChannels, RING_BUF_SIZE>  rc_data;
extern RingBuffer<uint32_t, RING_BUF_SIZE>    rc_timestamp;

extern RingBuffer<Vec4, RING_BUF_SIZE>        att_quat;      // Unit quaternion [q0, q1, q2, q3]
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> att_euler;     // Euler angles [roll, pitch, yaw] (deg)
extern RingBuffer<uint32_t, RING_BUF_SIZE>    att_timestamp; // Microseconds (us)


// Layer 3 Middleware Callbacks for NimBLE Server Events ([Note 0001])
class FcBleServerCallbacks : public NimBLEServerCallbacks {
public:
  void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override;
  void onDisconnect(NimBLEServer* pServer) override;
};

extern FcBleServerCallbacks ble_server_callbacks;

// Centralized Inter-Task Communication (IPC) Initialization Routine
bool init_ipc();


