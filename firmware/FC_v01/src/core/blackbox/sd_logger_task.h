#pragma once
#include <cstdint>

extern volatile bool sd_stop_logging_requested;

// Set by the logger when it can no longer run. Layer 5 owns the actual suspension
// (0042) — the task itself never suspends or deletes itself.
extern volatile bool sd_logger_halt_requested;

void sd_request_halt();   // idempotent; called from the logger task's own context

// Storage verdict published by the SD logger task (single writer).
// ADVISORY BY DESIGN: never gates flight - see zettel 0056.
enum class StorageVerdict : uint8_t {
  UNKNOWN  = 0,  // Logger has not evaluated the card yet (no verdict)
  HEALTHY,       // Mounted, log file open, free space above threshold
  DEGRADED       // Absent, unmountable, write-failing, full or below threshold
};

struct StorageHealthSnapshot {
  bool           mounted;     // SD card mounted (bool)
  bool           degraded;    // Logging unusable or failing (bool)
  uint64_t       bytes_free;  // Free space on the mounted card (bytes)
  StorageVerdict verdict;     // Tri-state advisory verdict
};

// Returns the latest storage health snapshot (never blocks, never gates flight).
StorageHealthSnapshot storage_health_snapshot();

void sd_logger_task(void* arg);
