#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 10
#define I2C_SCL 9

#define MPU6050_ADDR_0 0x68
#define MPU6050_ADDR_1 0x69
#define BMP085_ADDR 0x77

#define BMP085_CONTROL 0xF4
#define BMP085_TEMPDATA 0xF6
#define BMP085_PRESSUREDATA 0xF6
#define BMP085_READTEMPCMD 0x2E
#define BMP085_READPRESSURECMD 0x34

#define BMP085_ULTRALOWPOWER 0
#define BMP085_STANDARD 1

#define BMP085_CHIPID 0xD0
#define BMP085_EXPECTED_CHIPID 0x55

// Raw data variables for accelerometer, gyroscope, and temperature from the MPU6050 sensor
int16_t ax, ay, az;
int16_t tempRaw;
int16_t gx, gy, gz;

unsigned long previousImuMillis = 0;
unsigned long previousBaroMillis = 0;
uint8_t mpuAddress = MPU6050_ADDR_0;

int16_t bmpAc1;
int16_t bmpAc2;
int16_t bmpAc3;
uint16_t bmpAc4;
uint16_t bmpAc5;
uint16_t bmpAc6;
int16_t bmpB1;
int16_t bmpB2;
int16_t bmpMb;
int16_t bmpMc;
int16_t bmpMd;
uint8_t bmpOversampling = BMP085_ULTRALOWPOWER;
int32_t bmpB5 = 0;
int32_t bmpReferencePressurePa = 101325;
float barometerTemperatureC = 0.0f;
int32_t pressurePa = 0;
float relativeAltitudeM = 0.0f;

bool probeI2CDevice(uint8_t address)
{
  Wire.beginTransmission(address);
  return Wire.endTransmission(true) == 0;
}

uint8_t read8(uint8_t address, uint8_t registerAddress)
{
  Wire.beginTransmission(address);
  Wire.write(registerAddress);
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)address, (uint8_t)1, true);
  return Wire.read();
}

uint16_t read16(uint8_t address, uint8_t registerAddress)
{
  Wire.beginTransmission(address);
  Wire.write(registerAddress);
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)address, (uint8_t)2, true);
  return ((uint16_t)Wire.read() << 8) | Wire.read();
}

void write8(uint8_t address, uint8_t registerAddress, uint8_t value)
{
  Wire.beginTransmission(address);
  Wire.write(registerAddress);
  Wire.write(value);
  Wire.endTransmission(true);
}

bool beginBarometer()
{
  if (!probeI2CDevice(BMP085_ADDR))
  {
    return false;
  }

  if (read8(BMP085_ADDR, BMP085_CHIPID) != BMP085_EXPECTED_CHIPID)
  {
    return false;
  }

  bmpAc1 = (int16_t)read16(BMP085_ADDR, 0xAA);
  bmpAc2 = (int16_t)read16(BMP085_ADDR, 0xAC);
  bmpAc3 = (int16_t)read16(BMP085_ADDR, 0xAE);
  bmpAc4 = read16(BMP085_ADDR, 0xB0);
  bmpAc5 = read16(BMP085_ADDR, 0xB2);
  bmpAc6 = read16(BMP085_ADDR, 0xB4);
  bmpB1 = (int16_t)read16(BMP085_ADDR, 0xB6);
  bmpB2 = (int16_t)read16(BMP085_ADDR, 0xB8);
  bmpMb = (int16_t)read16(BMP085_ADDR, 0xBA);
  bmpMc = (int16_t)read16(BMP085_ADDR, 0xBC);
  bmpMd = (int16_t)read16(BMP085_ADDR, 0xBE);

  return true;
}

int32_t readBarometerPressurePa();

int32_t readBarometerPressureAverage(uint8_t samples)
{
  int64_t total = 0;
  for (uint8_t sample = 0; sample < samples; ++sample)
  {
    total += readBarometerPressurePa();
  }
  return (int32_t)(total / samples);
}

int32_t readBarometerRawTemperature()
{
  write8(BMP085_ADDR, BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(5);
  return read16(BMP085_ADDR, BMP085_TEMPDATA);
}

int32_t readBarometerRawPressure()
{
  write8(BMP085_ADDR, BMP085_CONTROL, BMP085_READPRESSURECMD + (bmpOversampling << 6));

  switch (bmpOversampling)
  {
  case BMP085_ULTRALOWPOWER:
    delay(5);
    break;
  case BMP085_STANDARD:
    delay(8);
    break;
  default:
    delay(14);
    break;
  }

  Wire.beginTransmission(BMP085_ADDR);
  Wire.write(BMP085_PRESSUREDATA);
  Wire.endTransmission(false);
  Wire.requestFrom((uint16_t)BMP085_ADDR, (uint8_t)3, true);

  uint32_t rawPressure = ((uint32_t)Wire.read() << 16) | ((uint16_t)Wire.read() << 8) | Wire.read();
  rawPressure >>= (8 - bmpOversampling);
  return (int32_t)rawPressure;
}

float updateBarometerTemperatureC()
{
  int32_t ut = readBarometerRawTemperature();
  int32_t x1 = ((ut - (int32_t)bmpAc6) * (int32_t)bmpAc5) >> 15;
  int32_t x2 = ((int32_t)bmpMc << 11) / (x1 + (int32_t)bmpMd);
  bmpB5 = x1 + x2;
  return ((bmpB5 + 8) >> 4) / 10.0f;
}

float pressureToAltitudeM(int32_t pressure)
{
  return 44330.0f * (1.0f - powf((float)pressure / (float)bmpReferencePressurePa, 0.1903f));
}

int32_t readBarometerPressurePa()
{
  int32_t up = readBarometerRawPressure();

  int32_t b6 = bmpB5 - 4000;
  int32_t x1;
  int32_t x2;
  x1 = ((int32_t)bmpB2 * ((b6 * b6) >> 12)) >> 11;
  x2 = ((int32_t)bmpAc2 * b6) >> 11;
  int32_t x3 = x1 + x2;
  int32_t b3 = (((((int32_t)bmpAc1 * 4 + x3) << bmpOversampling) + 2) >> 2);

  x1 = ((int32_t)bmpAc3 * b6) >> 13;
  x2 = ((int32_t)bmpB1 * ((b6 * b6) >> 12)) >> 16;
  x3 = ((x1 + x2) + 2) >> 2;
  uint32_t b4 = ((uint32_t)bmpAc4 * (uint32_t)(x3 + 32768)) >> 15;
  uint32_t b7 = ((uint32_t)(up - b3) * (uint32_t)(50000 >> bmpOversampling));

  int32_t pressure;
  if (b7 < 0x80000000)
  {
    pressure = (int32_t)((b7 << 1) / b4);
  }
  else
  {
    pressure = (int32_t)((b7 / b4) << 1);
  }

  x1 = (pressure >> 8) * (pressure >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * pressure) >> 16;
  pressure += (x1 + x2 + 3791) >> 4;
  return pressure;
}

void setup()
{
  Serial.begin(115200); // Start serial communication at 115200 baud rate

  Wire.begin(I2C_SDA, I2C_SCL); // Initialize I2C communication with specified SDA and SCL pins
  Wire.setClock(400000);        // Set I2C clock speed to 400 kHz

  delay(100); // Short delay to ensure sensor is ready

  if (probeI2CDevice(MPU6050_ADDR_0))
  {
    mpuAddress = MPU6050_ADDR_0;
  }
  else if (probeI2CDevice(MPU6050_ADDR_1))
  {
    mpuAddress = MPU6050_ADDR_1;
  }
  else
  {
    Serial.println("MPU6050 not found on I2C addresses 0x68 or 0x69");
    while (true)
    {
      delay(1000);
    }
  }

  if (!beginBarometer())
  {
    Serial.println("BMP085/BMP180 barometer not found on I2C address 0x77");
    while (true)
    {
      delay(1000);
    }
  }

  // ensure temperature compensation (bmpB5) is calculated before pressure averaging
  updateBarometerTemperatureC();
  bmpReferencePressurePa = readBarometerPressureAverage(8);

  // Wake up the MPU6050 sensor by writing to its power management register
  Wire.beginTransmission(mpuAddress); // Start communication with MPU6050 sensor
  Wire.write(0x6B);                   // Write to the power management register
  Wire.write(0x00);                   // Set to zero (wakes up the sensor)
  Wire.endTransmission(true);         // End transmission

  Wire.beginTransmission(mpuAddress); // Start communication with MPU6050 sensor
  Wire.write(0x1B);                   // Write to the gyroscope configuration register
  Wire.write(0x10);                   // Set gyroscope full scale range to ±1000 degrees per second
  Wire.endTransmission();             // End transmission

  Wire.beginTransmission(mpuAddress); // Start communication with MPU6050 sensor
  Wire.write(0x1C);                   // Write to the accelerometer configuration register
  Wire.write(0x10);                   // Set accelerometer full scale range to ±8g
  Wire.endTransmission();             // End transmission

  Wire.beginTransmission(mpuAddress); // Start communication with MPU6050 sensor
  Wire.write(0x1A);                   // Write to the digital low-pass filter configuration register
  Wire.write(0x02);                   // Set the digital low-pass filter to 44 Hz
  Wire.endTransmission();             // End transmission

  Serial.printf("MPU6050 initialized successfully at I2C address 0x%02X\n", mpuAddress); // Print initialization success message
  Serial.printf("BMP085/BMP180 barometer initialized successfully at I2C address 0x%02X\n", BMP085_ADDR);
  Serial.printf("Barometer baseline pressure: %ld Pa\n", (long)bmpReferencePressurePa);
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousImuMillis >= 10)
  {
    if (previousImuMillis == 0)
    {
      Serial.println("Starting data read from MPU6050...");
      Serial.println("AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,MPUTempC,BaroTempC,PressurePa,RelAltM,Us");
    }
    previousImuMillis = currentMillis;
    unsigned long currentMicros = micros();

    Wire.beginTransmission(mpuAddress); // Start communication with MPU6050 sensor
    Wire.write(0x3B);                   // Write to the starting register for accelerometer data
    Wire.endTransmission(false);        // End transmission but keep the connection active

    uint8_t bytesRead = Wire.requestFrom(mpuAddress, (uint8_t)14, (uint8_t)true); // Request 14 bytes of data from the sensor
    if (bytesRead != 14)
    {
      Serial.printf("I2C read failed, expected 14 bytes but got %u\n", bytesRead);
      return;
    }

    // Read accelerometer, temperature, and gyroscope data from the sensor
    ax = Wire.read() << 8 | Wire.read(); // Combine high and low bytes for X-axis acceleration
    ay = Wire.read() << 8 | Wire.read(); // Combine high and low bytes for Y-axis acceleration
    az = Wire.read() << 8 | Wire.read(); // Combine high and low bytes for Z-axis acceleration

    tempRaw = Wire.read() << 8 | Wire.read(); // Combine high and low bytes for temperature

    gx = Wire.read() << 8 | Wire.read(); // Combine high and low bytes for X-axis gyroscope
    gy = Wire.read() << 8 | Wire.read(); // Combine high and low bytes for Y-axis gyroscope
    gz = Wire.read() << 8 | Wire.read(); // Combine high and low bytes for Z-axis gyroscope

    float temperatureC = (tempRaw / 340.0f) + 36.53f;

    if (currentMillis - previousBaroMillis >= 50 || previousBaroMillis == 0)
    {
      previousBaroMillis = currentMillis;
      barometerTemperatureC = updateBarometerTemperatureC();
      pressurePa = readBarometerPressurePa();
      relativeAltitudeM = pressureToAltitudeM(pressurePa);
    }

    Serial.printf("%d,%d,%d,%d,%d,%d,%.3f,%.3f,%ld,%.2f,%lu\n",
                  ax, ay, az, gx, gy, gz, temperatureC, barometerTemperatureC, (long)pressurePa, relativeAltitudeM, micros() - currentMicros);
  }
}