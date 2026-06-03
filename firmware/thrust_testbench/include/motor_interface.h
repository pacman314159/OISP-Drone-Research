#pragma once

#include "Arduino.h"

#define PWM_FREQ_HZ 250
#define PWM_MAX_PULSE_LEN_US 2000
#define PWM_MIN_PULSE_LEN_US 1000

class Motor_Interface{
private:
  uint8_t channel;
  uint8_t resolution_bit;
  float coeff, offset;

public:
  uint32_t duty;

  void config(uint8_t channel,uint8_t resolution_bit){
    if(resolution_bit > 14) return;
    this->resolution_bit = resolution_bit;
    this->channel = channel;    

    float max_duty = (1 << this->resolution_bit) / (1e6 / PWM_FREQ_HZ) * PWM_MAX_PULSE_LEN_US;
    float min_duty = (1 << this->resolution_bit) / (1e6 / PWM_FREQ_HZ) * PWM_MIN_PULSE_LEN_US;
    this->coeff = (max_duty - min_duty) / (PWM_MAX_PULSE_LEN_US - PWM_MIN_PULSE_LEN_US);
    this->offset = (PWM_MIN_PULSE_LEN_US * max_duty - PWM_MAX_PULSE_LEN_US * min_duty) / (PWM_MAX_PULSE_LEN_US - PWM_MIN_PULSE_LEN_US);

    ledcSetup(this->channel, PWM_FREQ_HZ, resolution_bit);
  }

  void attach_gpio(uint8_t gpio){
    pinMode(gpio, OUTPUT);
    ledcAttachPin(gpio, this->channel);
  } 

  void update_pulse_len(int len){
    this->duty = constrain(len, PWM_MIN_PULSE_LEN_US, PWM_MAX_PULSE_LEN_US) * this->coeff - this->offset;
  }

  void drive(){
    ledcWrite(this->channel, this->duty);
  }

  void set_pwm_range(){
  }
};
