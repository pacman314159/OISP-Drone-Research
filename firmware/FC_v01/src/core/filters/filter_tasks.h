#pragma once

#include "config.h"
#include "lpf.h"

// Layer 4 Core Filter Task Runners ([Zettel 0008])
void accel_gyro_lpf_task(void* arg);
void mag_lpf_task(void* arg);
