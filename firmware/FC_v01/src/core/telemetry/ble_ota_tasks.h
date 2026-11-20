#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class HAL_OTA;

void ble_ota_task(void* arg);

// Spawns ble_ota_task dynamically on-demand (allocating queue if needed).
// [C7] The OTA HAL is injected by Layer 5 instead of fetched by Layer 4.
bool spawn_ble_ota_task(HAL_OTA& ota);
