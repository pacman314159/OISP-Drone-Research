#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Critical section macros

#if defined(ESP_PLATFORM) || defined(ESP32)
  extern portMUX_TYPE sys_spinlock;
  #define SYS_ENTER_CRITICAL() taskENTER_CRITICAL(&sys_spinlock)
  #define SYS_EXIT_CRITICAL()  taskEXIT_CRITICAL(&sys_spinlock)
#else
  #define SYS_ENTER_CRITICAL() taskENTER_CRITICAL()
  #define SYS_EXIT_CRITICAL()  taskEXIT_CRITICAL()
#endif
