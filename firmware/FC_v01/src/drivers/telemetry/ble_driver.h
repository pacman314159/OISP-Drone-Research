#pragma once
#include <cstdint>
#include <cstddef>

bool ble_driver_init();
bool send_vec3_raw_telemetry_notification(const uint8_t* data, size_t len);
bool send_vec3_filt_telemetry_notification(const uint8_t* data, size_t len);
bool is_ble_connected();
void set_ble_connected(bool connected);


