#pragma once
#include <cstdint>

// Application entry point for FSM setup and RTOS task spawning per [Note 0000]
void app_start();

// Unconditionally suspends all application tasks during active BLE OTA flashing
void app_suspend_all_tasks();

// Layer 5 authority (0042)
void app_halt_task(uint32_t task_id);
