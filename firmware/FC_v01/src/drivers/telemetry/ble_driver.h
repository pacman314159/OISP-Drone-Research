#pragma once
#include <cstdint>
#include <cstddef>

bool ble_driver_init();
bool is_ble_connected();
void set_ble_connected(bool connected);

// Telemetry Stream Notification & Subscription APIs
bool is_ble_stream_subscribed(uint16_t mask_bit);
bool send_ble_stream_notification(uint16_t mask_bit, const uint8_t* data, size_t len); // len: bytes

// Legacy Stream-Specific Forwarders (Preserved for compatibility)
bool send_ble_gyro_raw_notification(const uint8_t* data, size_t len);   // len: bytes (BleGyroPacket 16B payload)
bool send_ble_gyro_filt_notification(const uint8_t* data, size_t len);  // len: bytes (BleGyroPacket 16B payload)
bool send_ble_accel_raw_notification(const uint8_t* data, size_t len);  // len: bytes (BleAccelPacket 16B payload)
bool send_ble_accel_filt_notification(const uint8_t* data, size_t len); // len: bytes (BleAccelPacket 16B payload)
bool send_ble_mag_raw_notification(const uint8_t* data, size_t len);    // len: bytes (BleMagPacket 16B payload)
bool send_ble_mag_filt_notification(const uint8_t* data, size_t len);   // len: bytes (BleMagPacket 16B payload)
bool send_ble_baro_raw_notification(const uint8_t* data, size_t len);   // len: bytes (BleBaroPacket 8B payload)
bool send_ble_baro_filt_notification(const uint8_t* data, size_t len);  // len: bytes (BleBaroPacket 8B payload)
bool send_ble_att_quat_notification(const uint8_t* data, size_t len);   // len: bytes (BleAttQuatPacket 20B payload)
bool send_ble_att_euler_notification(const uint8_t* data, size_t len);  // len: bytes (BleAttEulerPacket 16B payload)

// OTA Update APIs
bool send_ble_ota_response(const uint8_t* data, size_t len); // len: bytes
bool is_ble_ota_active();
void set_ble_ota_active(bool active);
