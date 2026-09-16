#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// =============================================================================
// LAYER 3 MIDDLEWARE: SPINLOCK & CRITICAL SECTION OSAL ABSTRACTION ([Note 0002])
// =============================================================================

class Spinlock {
public:
  Spinlock(){
#if defined(ESP_PLATFORM) || defined(ESP32)
    _mux = portMUX_INITIALIZER_UNLOCKED;
#endif
  }

  void lock() const{
#if defined(ESP_PLATFORM) || defined(ESP32)
    taskENTER_CRITICAL(&_mux);
#else
    taskENTER_CRITICAL();
#endif
  }

  void unlock() const{
#if defined(ESP_PLATFORM) || defined(ESP32)
    taskEXIT_CRITICAL(&_mux);
#else
    taskEXIT_CRITICAL();
#endif
  }

private:
#if defined(ESP_PLATFORM) || defined(ESP32)
  mutable portMUX_TYPE _mux;
#endif
};

// RAII Guard for Exception-Safe & Early-Return-Safe Critical Sections
class CriticalGuard {
public:
  explicit CriticalGuard(const Spinlock& lock) : _lock(lock){
    _lock.lock();
  }

  ~CriticalGuard(){
    _lock.unlock();
  }

  CriticalGuard(const CriticalGuard&) = delete;
  CriticalGuard& operator=(const CriticalGuard&) = delete;

private:
  const Spinlock& _lock;
};
