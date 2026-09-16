#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Sensor DAQ Task Functions
void accel_gyro_daq_task(void* arg);
void mag_daq_task(void* arg);
void baro_daq_task(void* arg);
