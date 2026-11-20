#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "config.h"
#include "ipc_types.h"
#include "middleware/ring_buffer.h"
#include "platforms/hal/hal_matrix.h"

// =============================================================================
// LAYER 3 — SHARED HANDLES (declarations only; ipc.cpp is the single owner)
// =============================================================================
// Split out of ipc.h by C15. Include this header when you need a handle.
// Ownership and lifetime still live in middleware/ipc.cpp.

// System Mutex Handles (I2C Bus, SPI Bus, Flash & Estimator Locks)
extern SemaphoreHandle_t i2c0_mutex;
extern SemaphoreHandle_t i2c1_mutex;
extern SemaphoreHandle_t spi_bus_mutex;
extern SemaphoreHandle_t flash_mutex;
extern SemaphoreHandle_t ekf_state_mutex;

// System Task Notification Registry (Indexed by TaskIDs enum)
// Mutate through ipc_register_task() only — never write this array directly (C4).
extern TaskHandle_t app_task_handles[TASK_COUNT];

// Inter-Task Communication Queues
extern QueueHandle_t ota_data_queue;
extern QueueHandle_t led_cmd_queue;
extern QueueHandle_t calib_report_queue;
extern QueueHandle_t motor_cmd_queue;

// -----------------------------------------------------------------------------
// Thread-Safe Inter-Task Ring Buffers
//
// SPSC DISCIPLINE (C15 rule 3): every ring buffer below is single-producer /
// single-consumer and lock-free on the 500 Hz / 2 ms flight path. Direct
// access is sanctioned ONLY because push()/pop() use a short critical section
// (ring_buffer.h:18-25). Do NOT wrap these in a mutex-bearing API — added
// jitter on the flight loop is a reliability regression, not an improvement.
// -----------------------------------------------------------------------------

// Raw sensor streams
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> accel_raw;
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> gyro_raw;
extern RingBuffer<Vec3<float>, RING_BUF_SIZE> mag_raw;
extern RingBuffer<float, RING_BUF_SIZE>       pressure;

// Timestamps & filtered streams
extern RingBuffer<uint32_t, RING_BUF_SIZE>    raw_accel_gyro_timestamp;
extern RingBuffer<uint32_t, RING_BUF_SIZE>    raw_mag_timestamp;
extern RingBuffer<uint32_t, RING_BUF_SIZE>    pressure_timestamp;

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
