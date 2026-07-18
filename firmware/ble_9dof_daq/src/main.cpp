#include <Arduino.h>
#include <NimBLEDevice.h>
#include "GY87_MPU6050.h"
#include "GY87_HMC5883L.h"

// --- Constants ---
constexpr uint8_t PIN_SDA = 10;
constexpr uint8_t PIN_SCL = 9;
constexpr uint8_t PIN_DRDY = 7;
constexpr uint16_t SAMPLING_INTERVAL_US = 2000; // 500 Hz
constexpr size_t QUEUE_LENGTH = 60;

// UUIDs for BLE
// You can change these to custom UUIDs later if needed
#define NIMBLE_DEVICE_NAME "ESP32S3_Drone_IMU"
#define NIMBLE_SERVICE_UUID "d3a9f560-8f77-4a45-b3e0-3c22d8f23c91" // Generic custom service UUID
#define NIMBLE_CHAR_SENSOR_UUID "b4f61a92-4c35-4f93-b2e1-89a37f6a5fdd" // Generic custom characteristic UUID

// --- Data Structures ---
struct IMUData {
  uint32_t timestamp;
  float gx, gy, gz;
  float ax, ay, az;
};

// --- Globals ---
QueueHandle_t imuQueue;
NimBLECharacteristic* pSensorChar;
bool deviceConnected = false;

// --- BLE Server Callbacks ---
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("Client connected!");
    }

    void onDisconnect(NimBLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("Client disconnected!");
        // Restart advertising
        NimBLEDevice::startAdvertising();
    }
};

// --- Tasks ---
void imu_task(void* pvParameters) {
  GY87_MPU6050 mpu;
  GY87_HMC5883L hmc;

  // I2C and Sensor Initialization
  mpu.init(PIN_SDA, PIN_SCL);
  // mpu.enableBypass();
  mpu.setAccRange(AFS_SEL_4G);
  mpu.setGyroRange(FS_SEL_500);

  // hmc.init(PIN_SDA, PIN_SCL);
  // hmc.setMeasMode(SINGLE_MODE);
  // hmc.setAveraging(AVERAGING_1);
  // hmc.setOutputRate(RATE_75);
  // hmc.setBiasMode(BIAS_NORMAL);
  // hmc.setGain(FIELD_RANGE_0_88);
  // hmc.attachDRDYInterrupt(PIN_DRDY);

  delay(100);

  IMUData buffer;
  float temp; // Discarded
  uint32_t startTimeUs;

  Serial.println("IMU Task Started.");

  while (true) {
    startTimeUs = micros();
    buffer.timestamp = startTimeUs;

    mpu.getRawAll();
    mpu.getAllData(buffer.ax, buffer.ay, buffer.az, temp, buffer.gx, buffer.gy, buffer.gz);
    
    // while(not hmc.isDataReady()) yield();
    // hmc.getRawAll();
    // hmc.setMeasMode(SINGLE_MODE);
    // hmc.getAllData(buffer.mx, buffer.my, buffer.mz);

    // Send to queue, don't block if full
    if (xQueueSend(imuQueue, &buffer, 0) != pdPASS) {
      // Serial.println("Queue full, dropping sample");
    }

    while (micros() - startTimeUs < SAMPLING_INTERVAL_US) {
      vTaskDelay(1); // Small delay to prevent watchdog trigger while spinning
    }
  }
}

void ble_task(void* pvParameters) {
  // NimBLE Initialization
  NimBLEDevice::init(NIMBLE_DEVICE_NAME);
  NimBLEDevice::setMTU(512); // Max MTU just in case

  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService *pServiceIMU = pServer->createService(NIMBLE_SERVICE_UUID);
  pSensorChar = pServiceIMU->createCharacteristic(
      NIMBLE_CHAR_SENSOR_UUID, 
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );

  pServiceIMU->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(NIMBLE_SERVICE_UUID);
  pAdvertising->setName(NIMBLE_DEVICE_NAME);
  pAdvertising->start();

  Serial.println("BLE Task Started. Advertising...");

  const int BATCH_SIZE = 15; // 15 * 28 bytes = 420 bytes (fits in 512 MTU)
  IMUData batch[BATCH_SIZE];
  int batch_index = 0;

  while (true) {
    if (xQueueReceive(imuQueue, &batch[batch_index], portMAX_DELAY) == pdPASS) {
      batch_index++;
      
      if (batch_index >= BATCH_SIZE) {
        if (deviceConnected && pSensorChar->getSubscribedCount() > 0) {
          pSensorChar->setValue((uint8_t*)batch, sizeof(batch));
          pSensorChar->notify();
        }
        batch_index = 0;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) { delay(10); }

  Serial.println("\n--- Starting ESP32S3 IMU + NimBLE ---");
  
  log_i("Free Internal Heap: %d bytes", ESP.getFreeHeap());
  
  imuQueue = xQueueCreate(QUEUE_LENGTH, sizeof(IMUData));
  if (imuQueue == NULL) {
    Serial.println("Failed to create IMU Queue!");
    while(1) delay(100);
  }

  xTaskCreatePinnedToCore(ble_task, "BLETask", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(imu_task, "SensorTask", 4096, NULL, 2, NULL, 1);
}

void loop() {
  vTaskDelete(NULL); // Delete the Arduino loop task to save resources
}
