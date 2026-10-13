#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Critical section macros

#if defined(ESP_PLATFORM) || defined(ESP32)
  extern portMUX_TYPE sys_spinlock;
  #define SYS_ENTER_CRITICAL() taskENTER_CRITICAL(&sys_spinlock)
  #define SYS_EXIT_CRITICAL()  taskEXIT_CRITICAL(&sys_spinlock)
  #define SYS_ENTER_CRITICAL_ISR() taskENTER_CRITICAL_ISR(&sys_spinlock)
  #define SYS_EXIT_CRITICAL_ISR()  taskEXIT_CRITICAL_ISR(&sys_spinlock)
#else
  #define SYS_ENTER_CRITICAL() taskENTER_CRITICAL()
  #define SYS_EXIT_CRITICAL()  taskEXIT_CRITICAL()
  #define SYS_ENTER_CRITICAL_ISR() taskENTER_CRITICAL_FROM_ISR()
  #define SYS_EXIT_CRITICAL_ISR()  taskEXIT_CRITICAL_FROM_ISR(0)
#endif
