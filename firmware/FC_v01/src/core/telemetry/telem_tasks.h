#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void ble_vec3_raw_trans_task(void* arg);
void ble_vec3_filtered_trans_task(void* arg);
