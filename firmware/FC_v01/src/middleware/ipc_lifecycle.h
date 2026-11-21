#pragma once
#include <cstdint>
#include <cstddef>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// =============================================================================
// LAYER 3 — LIFECYCLE API (owner: ipc.cpp)
// =============================================================================
// Layer 5 and object owners use these; the objects
// themselves are still created/destroyed only in middleware/ipc.cpp.

// Centralized Inter-Task Communication (IPC) Initialization Routine
bool init_ipc();

// [0050] On-demand OTA queue. Allocated on first use, never at boot.
// Layer 4 supplies the payload size so this call needs no packet header.
bool ipc_create_ota_queue(size_t chunk_size);   // chunk_size: bytes per queue element
void ipc_destroy_ota_queue();

// -----------------------------------------------------------------------------
// Task registry accessors
//
// The registry feeds Layer 5's suspension authority (app_suspend_all_tasks).
// It is written from two layers today, so every mutation goes through here and
// is performed inside a critical section. Never touch app_task_handles[] directly.
// -----------------------------------------------------------------------------
void         ipc_register_task(uint32_t task_id, TaskHandle_t handle); // nullptr clears the slot
TaskHandle_t ipc_get_task(uint32_t task_id);
