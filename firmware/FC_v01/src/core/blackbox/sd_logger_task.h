#pragma once

extern volatile bool sd_stop_logging_requested;

// Set by the logger when it can no longer run. Layer 5 owns the actual suspension
// (0042) — the task itself never suspends or deletes itself.
extern volatile bool sd_logger_halt_requested;

void sd_request_halt();   // idempotent; called from the logger task's own context

void sd_logger_task(void* arg);
