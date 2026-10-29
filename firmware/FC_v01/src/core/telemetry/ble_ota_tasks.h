#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void ble_ota_task(void* arg);

// Spawns ble_ota_task dynamically (allocating queue if needed)
bool spawn_ble_ota_task(bool immediate_start);
