#pragma once
#include <cstdint>
#include <cstddef>

bool ble_driver_init();
bool is_ble_connected();
void set_ble_connected(bool connected);

// Telemetry Stream Notification & Subscription APIs
bool is_ble_stream_subscribed(uint16_t mask_bit);
bool send_ble_stream_notification(uint16_t mask_bit, const uint8_t* data, size_t len); // len: bytes

// OTA Update APIs
bool send_ble_ota_response(const uint8_t* data, size_t len); // len: bytes
bool is_ble_ota_active();
void set_ble_ota_active(bool active);

// OTA Trigger Authorization Callback (Dependency Inversion per [Note 0039])
typedef bool (*BleOtaTriggerCallback)();
void set_ble_ota_trigger_callback(BleOtaTriggerCallback cb);

// OTA Transport Port (Dependency Inversion, audit item C3).
// Layer 2 owns the NimBLE stack and reports transport events; Layer 3 owns the
// queue, the abort flag and the task wake-up. Layer 2 never touches IPC globals.
enum BleOtaEvent : uint8_t {
  BLE_OTA_EVT_DATA  = 0,  // a chunk arrived; data/len carry it
  BLE_OTA_EVT_EOF   = 1,  // transfer finished; data carries the 4-byte fw size
  BLE_OTA_EVT_ABORT = 2   // abort requested by the peer, or by an overflow
};

// Returns true when the chunk was accepted by the upper layer.
typedef bool (*BleOtaSink)(uint8_t evt, const uint8_t* data, uint16_t len);
void ble_ota_set_sink(BleOtaSink sink);
