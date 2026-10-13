#include "core/interface/user_button.h"
#include <Arduino.h>
#include "config.h"
#include "middleware/ipc.h"

void user_button_init(){
  pinMode(USER_BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(USER_BTN_PIN), user_btn_isr, CHANGE);
}

bool user_button_is_pressed(){
  return (digitalRead(USER_BTN_PIN) == LOW);
}
