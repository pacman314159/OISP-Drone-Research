#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_timer.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "middleware/ring_buffer.h"
#include "drivers/telemetry/ble_ota_packet.h"
#include "drivers/power/ina226.h"
#include "core/math/matrix.h"
#include "drivers/rc_rx/fs_ia6b.h"

// =============================================================================
// LAYER 3 MIDDLEWARE & CENTRALIZED INTER-TASK IPC
// =============================================================================

// System Mutex Handles (I2C Bus, SPI Bus, Flash & Estimator Locks)
extern SemaphoreHandle_t i2c0_mutex;
extern SemaphoreHandle_t i2c1_mutex;
extern SemaphoreHandle_t spi_bus_mutex;
extern SemaphoreHandle_t flash_mutex;
extern SemaphoreHandle_t ekf_state_mutex;

// System Task Notification Registry (Indexed by TaskIDs enum)
extern TaskHandle_t app_task_handles[TASK_COUNT];

// Inter-Task Communication Queues
extern QueueHandle_t ota_data_queue;
extern QueueHandle_t led_cmd_queue;
extern QueueHandle_t calib_report_queue;
extern QueueHandle_t motor_cmd_queue;

// Hardware Button Event Flags & Middleware ISR Handler ([Note 0001])
enum BtnEventMasks : uint8_t {
  BTN_EVT_NONE        = 0,
  BTN_EVT_SHORT_CLICK = (1 << 0),
  BTN_EVT_LONG_HOLD   = (1 << 1)
};

uint8_t ipc_pop_btn_events();
void user_btn_isr();

// Thread-Safe Inter-Task Ring Buffers (Raw Sensor Streams)
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> accel_raw;
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> gyro_raw;
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> mag_raw;
extern RingBuffer<float, RING_BUF_SIZE>       pressure;
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

extern RingBuffer<Vec3<float>, RING_BUF_SIZE> alt_state;     // Vertical state [z (m), v_z (m/s), a_z_inertial (m/s^2)]
extern RingBuffer<uint32_t, RING_BUF_SIZE>    alt_timestamp; // Microseconds (us)

// INDI Controller Shared Output Signal (Producer: Control Task, Consumer: Motor Task via SYS_ENTER_CRITICAL)
struct IndiControllerOutput {
  float       thrust_cmd{0.0f};          // Normalized collective throttle command [0.0, 1.0]
  Vec3<float> u_in{0.0f, 0.0f, 0.0f};    // Normalized 3-axis torque commands [roll (p), pitch (q), yaw (r)] in [-1.0, 1.0]
  uint32_t    timestamp_us{0};           // Update timestamp in microseconds (us)
  bool        is_armed{false};           // Safety arming flag
};

extern IndiControllerOutput indi_output;

// Actuator Commands & Direct Notification Bits for Layer 4 Actuator Task ([Note 0035])
enum MotorCommand : uint8_t {
  MOTOR_CMD_DISARM = 0,
  MOTOR_CMD_ARM,
  MOTOR_CMD_EMERGENCY_STOP
};

enum MotorNotificationBits : uint32_t {
  NOTIFY_MOTOR_NONE           = 0,
  NOTIFY_MOTOR_EMERGENCY_STOP = (1 << 0)
};

// Layer 3 Middleware Callbacks for NimBLE Server Events ([Note 0001])
class FcBleServerCallbacks : public NimBLEServerCallbacks {
public:
  void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override;
  void onDisconnect(NimBLEServer* pServer) override;
};

extern FcBleServerCallbacks ble_server_callbacks;

// Centralized Inter-Task Communication (IPC) Initialization Routine
bool init_ipc();
