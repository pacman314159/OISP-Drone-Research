#include <Arduino.h>

void setup() {
  Serial0.begin(115200);   
  delay(2000);

  Serial0.println("HELLO FROM SERIAL0");
}

void loop() {
  Serial0.println("RUNNING...");
  delay(1000);
}