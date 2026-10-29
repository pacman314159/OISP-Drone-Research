#include <Arduino.h>
#include <freertos/timers.h>
#include "middleware/ipc.h"
#include "middleware/critical_section.h"
#include "middleware/sysview_tracing.h"
#include "drivers/telemetry/ble_driver.h"
#include "core/interface/interface_tasks.h"
#include "core/sensor/calib.h"

SemaphoreHandle_t i2c0_mutex = nullptr;
SemaphoreHandle_t i2c1_mutex = nullptr;
SemaphoreHandle_t spi_bus_mutex = nullptr;
SemaphoreHandle_t flash_mutex = nullptr;
SemaphoreHandle_t ekf_state_mutex = nullptr;
TaskHandle_t app_task_handles[TASK_COUNT] = {nullptr};
QueueHandle_t ota_data_queue = nullptr;
QueueHandle_t led_cmd_queue = nullptr;
QueueHandle_t calib_report_queue = nullptr;

RingBuffer<Vec3<float>, RING_BUF_SIZE> accel_raw;
RingBuffer<Vec3<float>, RING_BUF_SIZE> gyro_raw;
RingBuffer<Vec3<float>, RING_BUF_SIZE> mag_raw;
RingBuffer<float, RING_BUF_SIZE>       pressure;
RingBuffer<PowerMeasurement, RING_BUF_SIZE> power_data;

RingBuffer<uint32_t, RING_BUF_SIZE>    raw_accel_gyro_timestamp;
RingBuffer<uint32_t, RING_BUF_SIZE>    raw_mag_timestamp;
RingBuffer<uint32_t, RING_BUF_SIZE>    pressure_timestamp;
RingBuffer<uint32_t, RING_BUF_SIZE>    power_timestamp;

RingBuffer<Vec3<float>, RING_BUF_SIZE> accel;
RingBuffer<Vec3<float>, RING_BUF_SIZE> gyro;
RingBuffer<Vec3<float>, RING_BUF_SIZE> mag;

RingBuffer<uint32_t, RING_BUF_SIZE>    filtered_accel_gyro_timestamp;
RingBuffer<uint32_t, RING_BUF_SIZE>    filtered_mag_timestamp;

RingBuffer<RcChannels, RING_BUF_SIZE>  rc_data;
RingBuffer<uint32_t, RING_BUF_SIZE>    rc_timestamp;

RingBuffer<Vec4, RING_BUF_SIZE>        att_quat;
RingBuffer<Vec3<float>, RING_BUF_SIZE> att_euler;
RingBuffer<uint32_t, RING_BUF_SIZE>    att_timestamp;

RingBuffer<Vec3<float>, RING_BUF_SIZE> alt_state;
RingBuffer<uint32_t, RING_BUF_SIZE>    alt_timestamp;

IndiControllerOutput indi_output = {};
QueueHandle_t motor_cmd_queue = nullptr;

FcBleServerCallbacks ble_server_callbacks;

void FcBleServerCallbacks::onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc){
  set_ble_connected(true);
  if(desc != nullptr)
    pServer->updateConnParams(desc->conn_handle, 6, 12, 0, 500);
  Serial.println("BLE Client Connected!");
}

void FcBleServerCallbacks::onDisconnect(NimBLEServer* pServer){
  set_ble_connected(false);

  if(is_ble_ota_active()){
    Serial.println("[BLE_OTA] Connection lost during active OTA! Triggering abort...");
    ota_abort_requested = true;
    if(ota_data_queue != nullptr){
      OtaDataChunk abort_chunk = {};
      abort_chunk.length = OTA_CHUNK_ABORT_MARKER;
      xQueueSend(ota_data_queue, &abort_chunk, 0);
    }
    if(app_task_handles[TASK_BLE_OTA_ID] != nullptr)
      xTaskNotifyGive(app_task_handles[TASK_BLE_OTA_ID]);
  }

  Serial.println("BLE Client Disconnected!");
  if(!is_ble_ota_active())
    NimBLEDevice::startAdvertising();
}

static TimerHandle_t s_btn_hold_timer = nullptr;
static volatile uint32_t s_btn_press_start_us = 0;
static volatile uint8_t s_btn_event_flags = BTN_EVT_NONE;

static void btn_hold_timer_callback(TimerHandle_t xTimer){
  if(digitalRead(USER_BTN_PIN) == LOW){
    Serial.println("[USER_BTN] 3-second hold detected! Posting BTN_EVT_LONG_HOLD");
    SYS_ENTER_CRITICAL();
    s_btn_event_flags |= BTN_EVT_LONG_HOLD;
    SYS_EXIT_CRITICAL();
    if(app_task_handles[TASK_FSM_ID] != nullptr)
      xTaskNotifyGive(app_task_handles[TASK_FSM_ID]);
  }
}

uint8_t ipc_pop_btn_events(){
  SYS_ENTER_CRITICAL();
  uint8_t evts = s_btn_event_flags;
  s_btn_event_flags = BTN_EVT_NONE;
  SYS_EXIT_CRITICAL();
  return evts;
}

void IRAM_ATTR user_btn_isr(){
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint32_t now_us = static_cast<uint32_t>(micros());

  if(digitalRead(USER_BTN_PIN) == LOW){
    s_btn_press_start_us = now_us;
    if(s_btn_hold_timer != nullptr)
      xTimerStartFromISR(s_btn_hold_timer, &xHigherPriorityTaskWoken);
  }else{
    if(s_btn_hold_timer != nullptr)
      xTimerStopFromISR(s_btn_hold_timer, &xHigherPriorityTaskWoken);

    uint32_t press_duration_us = now_us - s_btn_press_start_us;
    // Debounced short click: between 50 ms and 600 ms
    if(press_duration_us >= 50000 && press_duration_us <= 600000){
      SYS_ENTER_CRITICAL_ISR();
      s_btn_event_flags |= BTN_EVT_SHORT_CLICK;
      SYS_EXIT_CRITICAL_ISR();
      if(app_task_handles[TASK_FSM_ID] != nullptr)
        vTaskNotifyGiveFromISR(app_task_handles[TASK_FSM_ID], &xHigherPriorityTaskWoken);
    }
  }

  if(xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

bool init_ipc(){
  sysview_init_markers();

  if(i2c0_mutex == nullptr)
    i2c0_mutex = xSemaphoreCreateMutex();

  if(i2c1_mutex == nullptr)
    i2c1_mutex = xSemaphoreCreateMutex();

  if(spi_bus_mutex == nullptr)
    spi_bus_mutex = xSemaphoreCreateMutex();

  if(flash_mutex == nullptr)
    flash_mutex = xSemaphoreCreateMutex();

  if(ekf_state_mutex == nullptr)
    ekf_state_mutex = xSemaphoreCreateMutex();

  if(ota_data_queue == nullptr)
    ota_data_queue = xQueueCreate(OTA_DATA_QUEUE_DEPTH, sizeof(OtaDataChunk));

  if(led_cmd_queue == nullptr)
    led_cmd_queue = xQueueCreate(LED_CMD_QUEUE_DEPTH, sizeof(LedCommand));

  if(calib_report_queue == nullptr)
    calib_report_queue = xQueueCreate(CALIB_REPORT_QUEUE_DEPTH, sizeof(CalibReportMsg));

  if(motor_cmd_queue == nullptr)
    motor_cmd_queue = xQueueCreate(MOTOR_CMD_QUEUE_SIZE, sizeof(MotorCommand));

  if(s_btn_hold_timer == nullptr){
    s_btn_hold_timer = xTimerCreate(
      "BtnHoldTmr",
      pdMS_TO_TICKS(USER_BTN_HOLD_DURATION_MS),
      pdFALSE,
      nullptr,
      btn_hold_timer_callback
    );
  }

  return (i2c0_mutex != nullptr &&
          i2c1_mutex != nullptr &&
          spi_bus_mutex != nullptr &&
          flash_mutex != nullptr &&
          ekf_state_mutex != nullptr &&
          ota_data_queue != nullptr &&
          led_cmd_queue != nullptr &&
          calib_report_queue != nullptr &&
          motor_cmd_queue != nullptr &&
          s_btn_hold_timer != nullptr);
}
