#include "drivers/rc_rx/fs_ia6b.h"
#include <Arduino.h>

FS_iA6B::FS_iA6B(HAL_PulseRX& pulse_rx)
  : _pulse_rx(pulse_rx), _last_channels{}, _last_frame_us(0){
  for(uint8_t i = 0; i < RC_MAX_CHANNELS; ++i)
    _last_channels.channels[i] = PPM_CENTER_CHANNEL_US;
  _last_channels.channel_count = 6;
  _last_channels.failsafe = true;
}

bool FS_iA6B::init(int gpio_pin){
  bool ok = _pulse_rx.init(gpio_pin, 1000000, 3000);
  if(ok)
    _pulse_rx.start();
  return ok;
}

bool FS_iA6B::update(RcChannels& out_channels, uint32_t timeout_ms){
  uint32_t pulses[RC_MAX_CHANNELS + 2];
  size_t count = _pulse_rx.read_pulses(pulses, RC_MAX_CHANNELS + 2, timeout_ms);
  uint32_t now_us = static_cast<uint32_t>(micros());

  if(count >= 4){
    uint8_t valid_count = 0;
    for(size_t i = 0; i < count && valid_count < RC_MAX_CHANNELS; ++i){
      uint32_t p = pulses[i];
      if(p >= PPM_MIN_CHANNEL_US && p <= PPM_MAX_CHANNEL_US){
        _last_channels.channels[valid_count] = static_cast<uint16_t>(p);
        valid_count++;
      }
    }

    if(valid_count >= 4){
      _last_channels.channel_count = valid_count;
      _last_channels.failsafe = false;
      _last_frame_us = now_us;
      out_channels = _last_channels;
      return true;
    }
  }

  if((now_us - _last_frame_us) > PPM_FAILSAFE_TIMEOUT_US)
    _last_channels.failsafe = true;

  out_channels = _last_channels;
  return !_last_channels.failsafe;
}

bool FS_iA6B::is_failsafe() const{
  return _last_channels.failsafe;
}
