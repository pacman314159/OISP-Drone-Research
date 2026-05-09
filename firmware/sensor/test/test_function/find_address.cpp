#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 10
#define SCL_PIN 9

#define MPU_ADDR 0x68

// ---------- I2C SCANNER ----------
void scanI2C() {
  Serial.println("Scanning...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found: 0x");
      Serial.println(addr, HEX);
    }
  }
  Serial.println("Done\n");
}

// ---------- READ REGISTER ----------
uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 1);
  return Wire.read();
}

// ---------- ENABLE BYPASS ----------
void enableBypass() {
  Serial.println("Enabling MPU6050 bypass...");

  // Step 1: Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);   // PWR_MGMT_1
  Wire.write(0x00);   // Wake up
  Wire.endTransmission();
  delay(50);

  // Step 2: Disable I2C Master (VERY IMPORTANT)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6A);   // USER_CTRL
  Wire.write(0x00);   // I2C_MST_EN = 0
  Wire.endTransmission();
  delay(50);

  // Step 3: Enable bypass mode
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x37);   // INT_PIN_CFG
  Wire.write(0x02);   // I2C_BYPASS_EN = 1
  Wire.endTransmission();
  delay(50);

  Serial.println("Bypass enabled!\n");
}

// ---------- DEBUG REGISTERS ----------
void printMPUStatus() {
  uint8_t int_cfg = readRegister(0x37);
  uint8_t user_ctrl = readRegister(0x6A);

  Serial.print("INT_PIN_CFG (0x37): ");
  Serial.println(int_cfg, BIN);

  Serial.print("USER_CTRL (0x6A): ");
  Serial.println(user_ctrl, BIN);

  Serial.println();
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(2000);

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("=== BEFORE BYPASS ===");
  scanI2C();   // expect: 0x68, 0x77

  enableBypass();

  Serial.println("=== REGISTER CHECK ===");
  printMPUStatus();

  Serial.println("=== AFTER BYPASS ===");  
  delay(500);
  scanI2C();   // expect: 0x1E or 0x0D if magnetometer exists
}

// ---------- LOOP ----------
void loop() {
  scanI2C();
  delay(500);
}