#include "platforms/targets/espressif/pwm_ledc.h"
#include <Arduino.h>

HAL_PWM& get_pwm_hal(){
  static ESP32_PWM_LEDC pwm_inst;
  return pwm_inst;
}

bool ESP32_PWM_LEDC::init_timer(uint32_t freq_hz, uint8_t resolution_bits){
  if(resolution_bits > 14) resolution_bits = 14; // ESP32-S3 max hardware LEDC resolution is 14 bits
  if(freq_hz == 0) return false;

  _freq_hz = freq_hz;
  _resolution_bits = resolution_bits;

  ledc_timer_config_t timer_conf = {};
  timer_conf.speed_mode      = LEDC_LOW_SPEED_MODE;
  timer_conf.duty_resolution = static_cast<ledc_timer_bit_t>(_resolution_bits);
  timer_conf.timer_num       = LEDC_TIMER_0;
  timer_conf.freq_hz         = _freq_hz;
  timer_conf.clk_cfg         = LEDC_AUTO_CLK;

  esp_err_t err = ledc_timer_config(&timer_conf);
  if(err != ESP_OK) return false;

  _timer_initialized = true;
  return true;
}

bool ESP32_PWM_LEDC::init_channel(uint8_t channel_idx, int gpio_pin, uint32_t initial_duty_ticks){
  if(!_timer_initialized || channel_idx >= 8 || gpio_pin < 0) return false;

  ledc_channel_config_t ch_conf = {};
  ch_conf.gpio_num   = gpio_pin;
  ch_conf.speed_mode = LEDC_LOW_SPEED_MODE;
  ch_conf.channel    = static_cast<ledc_channel_t>(channel_idx);
  ch_conf.intr_type  = LEDC_INTR_DISABLE;
  ch_conf.timer_sel  = LEDC_TIMER_0;
  ch_conf.duty       = initial_duty_ticks;
  ch_conf.hpoint     = 0;

  esp_err_t err = ledc_channel_config(&ch_conf);
  if(err != ESP_OK) return false;

  _channel_initialized[channel_idx] = true;
  return true;
}

bool ESP32_PWM_LEDC::set_duty_ticks(uint8_t channel_idx, uint32_t duty_ticks){
  if(channel_idx >= 8 || !_channel_initialized[channel_idx]) return false;

  uint32_t max_ticks = (1UL << _resolution_bits) - 1UL;
  if(duty_ticks > max_ticks) duty_ticks = max_ticks;

  esp_err_t err_set = ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_idx), duty_ticks);
  if(err_set != ESP_OK) return false;

  esp_err_t err_upd = ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_idx));
  return (err_upd == ESP_OK);
}

bool ESP32_PWM_LEDC::set_pulse_us(uint8_t channel_idx, uint16_t pulse_us){
  if(channel_idx >= 8 || !_channel_initialized[channel_idx]) return false;

  // Convert pulse duration in microseconds to LEDC timer ticks:
  // ticks = (pulse_us * freq_hz * 2^resolution_bits) / 1,000,000
  uint64_t max_ticks = 1ULL << _resolution_bits;
  uint32_t duty_ticks = static_cast<uint32_t>((static_cast<uint64_t>(pulse_us) * _freq_hz * max_ticks) / 1000000ULL);

  return set_duty_ticks(channel_idx, duty_ticks);
}

bool ESP32_PWM_LEDC::stop_channel(uint8_t channel_idx){
  if(channel_idx >= 8 || !_channel_initialized[channel_idx]) return false;

  esp_err_t err = ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_idx), 0);
  return (err == ESP_OK);
}

