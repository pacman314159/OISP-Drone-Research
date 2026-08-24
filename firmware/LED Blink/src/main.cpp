#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.println("LED Blink");
  pinMode(6, OUTPUT);
}

void loop() {
  delay(2000);
  digitalWrite(6, LOW);
  delay(2000);
  digitalWrite(6, HIGH);
}
