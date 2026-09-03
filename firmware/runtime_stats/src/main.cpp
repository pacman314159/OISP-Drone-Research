#include <Arduino.h>
#include "freertos/task.h"
#include "TaskStatsAnalyzer.h"

// -----------------------------------------------------------------------------
// Example workload task
// This task simulates CPU usage so we can observe core utilization
// -----------------------------------------------------------------------------
void workTask(void *arg)
{
  // Volatile prevents compiler optimization removing the busy loop
  volatile uint32_t sink = 0;

  while (true)
  {

    // Busy loop to burn CPU cycles
    for (int i = 0; i < 200000; ++i)
    {
      sink += i;
    }

    // Yield CPU time for ~20 ms
    // Without this, the task would starve lower-priority tasks
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

TaskStatsAnalyzer taskStatsAnalyzer(5000);

void setup()
{
  Serial.begin(115200);
  delay(500);

  // Create a worker task pinned to core 0
  xTaskCreatePinnedToCore(
      workTask, // Task function
      "WORK0",  // Task name (shows up in stats)
      8192,     // Stack size in bytes
      nullptr,  // Task parameters
      2,        // Priority
      nullptr,  // Task handle
      0);       // Core 0

  // Create a worker task pinned to core 1
  xTaskCreatePinnedToCore(
      workTask,
      "WORK1",
      4096,
      nullptr,
      2,
      nullptr,
      1); // Core 1

  // Create a worker task with no affinity
  xTaskCreatePinnedToCore(
      workTask,
      "WORK2",
      4096,
      nullptr,
      2,
      nullptr,
      tskNO_AFFINITY);

  // Create a worker task with no affinity
  xTaskCreate(
      workTask,
      "WORK3",
      4096,
      nullptr,
      2,
      nullptr);
}

void loop()
{
  Serial.print(".");
  vTaskDelay(pdMS_TO_TICKS(10));
}
