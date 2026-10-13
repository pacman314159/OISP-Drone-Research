#pragma once

// Application entry point for FSM setup and RTOS task spawning per [Note 0000]
void app_start();

// Unconditionally suspends all application tasks during active BLE OTA flashing
void app_suspend_all_tasks();
