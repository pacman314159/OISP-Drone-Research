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
