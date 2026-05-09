#include <Arduino.h>
#include <Wire.h>
#include "GY87_BMP180.h"

int32_t debug_X1_P1;
int32_t debug_X2_P1;
int32_t debug_X1_P2;
int32_t debug_X2_P2;
int32_t debug_X1_P3;
int32_t debug_X2_P3;
int32_t debug_X1_P4;
int32_t debug_X1_T;
int32_t debug_X2_T;
int32_t debug_X3_P1;
int32_t debug_X3_P2;
int32_t debug_b3;
uint32_t debug_b4;
int32_t debug_b6;
int32_t press;
uint32_t debug_b7;

// Updating time variables
uint32_t currentTime = 0;
uint32_t sampleCount = 0;
const uint32_t sampleRate = 1000; 

// Calculate true temperature
int32_t calculateTT(long UT){
  int32_t X1, X2, T;

  X1 = ((UT - ac6) * ac5 ) >> 15;
  X2 = (mc << 11)/(X1 + md);
  b5 = X1 + X2;
  T = (b5 + 8) >> 4;
  return T;
}

// Calculate true pressure
int32_t calculateTP(long UP){
  int32_t X1, X2, X3, b3, b6, P;
  uint32_t b4;
  int64_t b7;

readCalibrationData(); 
calculateTT(UT); 

b6 = b5 - 4000; 
debug_b6 = b6; 
//read value b6 into global variable 
X1 = (b2 * ((b6 * b6) >> 12)) >> 11; 
X2 = (ac2 * b6) >> 11; X3 = X1 + X2; 

debug_X1_P1= X1; 
debug_X2_P1 = X2; 
debug_X3_P1 = X3; b3 = 

(((ac1 * 4 + X3) << oss) + 2) / 4; 
debug_b3 = b3; 
//read value b3 into global variable 
X1 = (ac3 * b6) >> 13; 
X2 = (b1 * ((b6 * b6) >> 12)) >> 16; 
X3 = ((X1 + X2) + 2) >> 2; 
b4 = (ac4 * ((unsigned long)(X3 + 32768))) >> 15; 
b7 = ((unsigned long)UP - b3)*(50000 >> oss); 
debug_b4 = b4; 
debug_b7 = b7; 

debug_X1_P2 = X1; 
debug_X2_P2= X2; 
debug_X3_P2 = X3; 

if (b7 < 0x80000000){ 
  P = (b7*2)/b4; } 
else{ P = (b7/b4)*2;
 }
  
up = P; 

X1 = (P >> 8) * (P >> 8); 

debug_X1_P3 = X1; 

X1 = (X1 * 3038) >> 16; 

debug_X1_P4 = X1; 

X2 = (-7357 * P) >> 16; 

debug_X2_P3 = X2; 

P = P + ((X1 + X2 + 3791) >> 4); 

return P;
}

// Calculate altitude from pressure
float barometerAltitude(int32_t pressure){
  altitudeBarometer = 44330*(1.0 - pow((pressure/101325.0),(1.0/5.255)))*100.00;
  return altitudeBarometer; // Placeholder return
}

// Calculate altitude from pressure with startup calibration
void setup(){
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  Wire.setClock(3400000);
  Wire.begin();
  delay(10);

  readCalibrationData();
}

void loop(){
  uint32_t tempTime = millis();

  uint32_t t0, t1;
  uint32_t timeUT, timeUP, timeTemp, timePre, timeAlt;

  /*if (sampleCount >= sampleRate){
    return;
  }*/

  if (tempTime - currentTime >= 2000){
    t0 = micros();
    long UT = readUT();
    t1 = micros();
    timeUT = t1 - t0;

    t0 = micros();
    int32_t temp = calculateTT(UT);
    t1 = micros();
    timeTemp = t1 - t0;

    Serial.print("Temperature: ");
    Serial.print(temp / 10.0);
    Serial.println(" °C");
    Serial.print("UT: ");
    Serial.println(UT);
  }

  Serial.print("AC1 = ");
  Serial.println(ac1);

  Serial.print("AC2 = ");
  Serial.println(ac2);

  Serial.print("AC3 = ");
  Serial.println(ac3);  

  Serial.print("AC4 = ");
  Serial.println(ac4);

  Serial.print("AC5 = ");
  Serial.println(ac5);

  Serial.print("AC6 = ");
  Serial.println(ac6);

  Serial.print("B1 = ");
  Serial.println(b1);

  Serial.print("B2 = ");
  Serial.println(b2);

  Serial.print("MB = ");
  Serial.println(mb);

  Serial.print("MC = ");
  Serial.println(mc);

  Serial.print("MD = ");
  Serial.println(md);

  Serial.print("b6 = ");
  Serial.println(debug_b6);

  Serial.print("X1 for p= ");
  Serial.println(debug_X1_P1);

  Serial.print("X2 for p= ");
  Serial.println(debug_X2_P1);

  Serial.print("X3 = ");
  Serial.println(debug_X3_P1);

  Serial.print("b2 = ");
  Serial.println(b2);

  Serial.print("b3 = ");
  Serial.println(debug_b3);

  Serial.print("X1 for p= ");
  Serial.println(debug_X1_P2);

  Serial.print("X2 for p= ");
  Serial.println(debug_X2_P2);
  
  Serial.print("X3 = ");
  Serial.println(debug_X3_P2);

  Serial.print("b4 = ");
  Serial.println(debug_b4);

  Serial.print("b5 = ");
  Serial.println(b5);

  Serial.print("b7 = ");
  Serial.println(debug_b7);

  Serial.print("P = ");
  Serial.println(up);

  Serial.print("X1 for p= ");
  Serial.println(debug_X1_P3);

  Serial.print("X1 for p= ");
  Serial.println(debug_X1_P4);
  
  Serial.print("X2 = ");
  Serial.println(debug_X2_P3);

  Serial.print("X1 for t= ");
  Serial.println(debug_X1_T);

  Serial.print("X2 for t= ");
  Serial.println(debug_X2_T);

  t0 = micros();
  long UP = readUP();
  t1 = micros();
  timeUP = t1 - t0;

  t0 = micros();
  int32_t pre = calculateTP(UP);
  t1 = micros();
  timePre = t1 - t0;

  t0 = micros();
  barometerAltitude(pre);
  t1 = micros();
  timeAlt = t1 - t0;

  Serial.print("UP: ");
  Serial.println(UP);

  Serial.print("Pressure: ");
  Serial.print(pre);
  Serial.println(" Pa");  
  Serial.print("Altitude:  ");
  Serial.print(altitudeBarometer);
  Serial.println(" cm");
  delay(10000);
   //--- Print timing ---
  Serial.println("--- Timing ---");
  if (tempTime - currentTime >= 2000){
    Serial.print("readUT:       "); Serial.print(timeUT);    Serial.println(" us");
    Serial.print("calculateTT:  "); Serial.print(timeTemp);  Serial.println(" us");
    Serial.print("readUP:       "); Serial.print(timeUP);    Serial.println(" us");
    Serial.print("calculateTP:  "); Serial.print(timePre); Serial.println(" us");
    Serial.print("altCalc:      "); Serial.print(timeAlt);   Serial.println(" us");
  
    Serial.print("TOTAL:        "); Serial.print(timeUT + timeUP + timeTemp + timePre + timeAlt);
    Serial.println(" us");
    Serial.println("==============");

    currentTime = tempTime;
  }
  else{
  Serial.print("readUP:       "); Serial.print(timeUP);    Serial.println(" us");
  Serial.print("calculateTP:  "); Serial.print(timePre); Serial.println(" us");
  Serial.print("altCalc:      "); Serial.print(timeAlt);   Serial.println(" us");
  
  Serial.print("TOTAL:        "); Serial.print(timeUP + timeTemp + timePre + timeAlt);
  Serial.println(" us");
  Serial.println("==============");
  }
  Serial.println(altitudeBarometer);
  //sampleCount++;
}
