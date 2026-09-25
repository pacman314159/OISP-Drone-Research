#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "config.h"
#include "middleware/critical_section.h"

template <typename T, size_t N>
class RingBuffer {
  static_assert(N <= RING_BUF_MAX_SIZE, "RingBuffer size exceeds RING_BUF_MAX_SIZE defined in config.h!");
  static_assert(N > 0, "RingBuffer size must be greater than zero!");

public:
  RingBuffer() : _head(0), _count(0){}

  void push(const T& sample){
    SYS_ENTER_CRITICAL();
    _buffer[_head] = sample;
    _head++;
    if(_head >= N) _head = 0;
    if(_count < N) _count++;
    SYS_EXIT_CRITICAL();
  }

  size_t get_snapshot(T out_window[N]){
    SYS_ENTER_CRITICAL();
    size_t count_copy = _count;
    if(count_copy == 0){
      SYS_EXIT_CRITICAL();
      return 0;
    }

    copy_window_internal(out_window, count_copy, _head);
    SYS_EXIT_CRITICAL();
    return count_copy;
  }

  size_t get_snapshot_then_clear(T out_window[N]){
    SYS_ENTER_CRITICAL();
    size_t count_copy = _count;
    if(count_copy == 0){
      SYS_EXIT_CRITICAL();
      return 0;
    }

    copy_window_internal(out_window, count_copy, _head);

    _head = 0;
    _count = 0;
    SYS_EXIT_CRITICAL();
    return count_copy;
  }

  inline T front() const {
    SYS_ENTER_CRITICAL();
    T sample = _buffer[_head];
    SYS_EXIT_CRITICAL();
    return sample;
  }

  inline size_t count() const {
    SYS_ENTER_CRITICAL();
    size_t val = _count;
    SYS_EXIT_CRITICAL();
    return val;
  }

  inline bool empty() const {
    SYS_ENTER_CRITICAL();
    bool val = (_count == 0);
    SYS_EXIT_CRITICAL();
    return val;
  }

  void clear(){
    SYS_ENTER_CRITICAL();
    _head = 0;
    _count = 0;
    SYS_EXIT_CRITICAL();
  }

private:
  inline void copy_window_internal(T out_window[N], size_t count_copy, size_t head_copy){
    if(count_copy < N)
      memcpy(out_window, _buffer, count_copy * sizeof(T));
    else{
      size_t right_len = N - head_copy;
      memcpy(out_window, &_buffer[head_copy], right_len * sizeof(T));
      if(head_copy > 0)
        memcpy(&out_window[right_len], &_buffer[0], head_copy * sizeof(T));
    }
  }

  T _buffer[N];
  size_t _head;
  size_t _count;
};
