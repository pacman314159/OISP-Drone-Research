#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// A dummy task that consumes a little bit of CPU
void dummyTask(void *arg) {
    while (1) {
        // Burn some CPU cycles
        for (volatile int i = 0; i < 50000; i++) {}
        // Yield to allow other tasks to run
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Create 5 dummy tasks spread across the dual-core system
    xTaskCreatePinnedToCore(dummyTask, "Task_1", 2048, NULL, 1, NULL, tskNO_AFFINITY); // Core 0
    xTaskCreatePinnedToCore(dummyTask, "Task_2", 2048, NULL, 1, NULL, tskNO_AFFINITY); // Core 1
    xTaskCreatePinnedToCore(dummyTask, "Task_3", 2048, NULL, 1, NULL, tskNO_AFFINITY); // Core 0
    xTaskCreatePinnedToCore(dummyTask, "Task_4", 2048, NULL, 1, NULL, tskNO_AFFINITY); // Core 1
    xTaskCreatePinnedToCore(dummyTask, "Task_5", 2048, NULL, 1, NULL, tskNO_AFFINITY); // No Affinity
}

void loop() {
    // Allocate a buffer large enough to hold the text. 
    // FreeRTOS recommends ~40 bytes per task. 1024 is plenty for a typical project.
    char statsBuffer[1024];
    
    // Call the built-in FreeRTOS formatting function
    vTaskGetRunTimeStats(statsBuffer);

    Serial.println("\n--- FreeRTOS Task Runtime Statistics ---");
    Serial.println("Task            Abs Time      % Time");
    Serial.println("----------------------------------------");
    
    // Print the raw formatted buffer
    Serial.print(statsBuffer);
    
    // Print out the statistics every 2 seconds
    delay(2000);
}

// Required ESP-IDF entry point for Arduino-as-a-component
extern "C" void app_main() {
    initArduino();
    setup();
    while(true) {
        loop();
    }
}
