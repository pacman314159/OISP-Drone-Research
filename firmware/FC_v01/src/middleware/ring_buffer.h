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
  RingBuffer() : _head(0), _count(0){
    memset(_buffer, 0, sizeof(_buffer));
  }

  void push(const T& sample){
    SYS_ENTER_CRITICAL();
    _buffer[_head] = sample;
    _head = (_head + 1) % N;
    if(_count < N) _count++;
    SYS_EXIT_CRITICAL();
  }

  bool get_snapshot(T out_window[N]){
    SYS_ENTER_CRITICAL();
    if(_count == 0){
      SYS_EXIT_CRITICAL();
      return false;
    }

    size_t count_copy = _count;
    size_t head_copy = _head;

    if(count_copy < N){ // Buffer not full yet, copy elements 0..count_copy-1
      memcpy(out_window, _buffer, count_copy * sizeof(T));
    } else { // Buffer full, copy in two chunks around the head index
      size_t right_len = N - head_copy;
      memcpy(out_window, &_buffer[head_copy], right_len * sizeof(T));
      if(head_copy > 0)
        memcpy(&out_window[right_len], &_buffer[0], head_copy * sizeof(T));
    }
    SYS_EXIT_CRITICAL();
    return true;
  }

  size_t count(){
    SYS_ENTER_CRITICAL();
    size_t c = _count;
    SYS_EXIT_CRITICAL();
    return c;
  }

  bool empty(){
    return (count() == 0);
  }

  void clear(){
    SYS_ENTER_CRITICAL();
    _head = 0;
    _count = 0;
    memset(_buffer, 0, sizeof(_buffer));
    SYS_EXIT_CRITICAL();
  }

private:
  T _buffer[N];
  size_t _head;
  size_t _count;
};
