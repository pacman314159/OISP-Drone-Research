#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Unified Telemetry Dispatcher Task (100 Hz Evaluation Cadence per Note 0021)
void ble_telem_task(void* arg);
