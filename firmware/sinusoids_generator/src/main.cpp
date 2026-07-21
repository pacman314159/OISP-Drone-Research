#include <Arduino.h>
#include <math.h>
#include <NimBLEDevice.h>

const float FREQUENCY = 1.0;
const float FREQUENCY2 = 10.0;
const float AMPLITUDE = 100.0;
const float AMPLITUDE2 = 50.0;

const float SAMPLE_RATE = 500.0;
const uint32_t SAMPLING_INTERVAL_US = (uint32_t)(1000000.0 / SAMPLE_RATE);

// New random UUIDs
#define NIMBLE_DEVICE_NAME "Sinusoid_Generator"
#define NIMBLE_SERVICE_UUID "14144463-7eb9-408a-b8ff-df2008efde13"
#define NIMBLE_CHAR_SENSOR_UUID "8b082129-9e8c-4a3f-85d7-ecf9273f55ba"

// Struct for the sequence number and two floats (12 bytes total)
struct SensorData {
  uint32_t timestamp;
  float value1;
  float value2;
};

// FreeRTOS Queue and BLE globals
QueueHandle_t sensorQueue;
NimBLECharacteristic* pSensorChar;
bool deviceConnected = false;

// Callbacks to track connection state
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("Client connected!");
    }

    void onDisconnect(NimBLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("Client disconnected!");
        NimBLEDevice::startAdvertising(); // Resume advertising when disconnected
    }
};

// Task 1: Generate Sinusoids using vTaskDelayUntil
void sensor_task(void* pvParameters) {
  SensorData data;
  data.timestamp = 0;
  
  uint32_t dropped_packets = 0;
  uint32_t loops_per_5s = 0;
  unsigned long lastLogTime = millis();

  Serial.println("Sensor Task Started.");

  // Convert sampling frequency into FreeRTOS ticks (assumes 1000Hz tick rate by default on ESP32)
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(1000.0 / SAMPLE_RATE);

  while (true) {
    float timeSeconds = millis() / 1000.0;
    
    // Generate dummy sinusoid data with timestamp
    data.timestamp = millis();
    data.value1 = AMPLITUDE * sin(2 * PI * FREQUENCY * timeSeconds);
    data.value2 = AMPLITUDE2 * sin(2 * PI * FREQUENCY2 * timeSeconds);

    // Send to queue, don't block if full
    if (xQueueSend(sensorQueue, &data, 0) != pdPASS) {
      dropped_packets++;
    }

    loops_per_5s++;

    // Print stats every 5 seconds
    if (millis() - lastLogTime >= 5000) {
      Serial.printf("Sensor Stats (last 5s) -> Dropped: %u | Loop Count: %u\n", dropped_packets, loops_per_5s);
      dropped_packets = 0;
      loops_per_5s = 0;
      lastLogTime = millis();
    }

    // Precise loop timing using FreeRTOS vTaskDelayUntil
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// Task 2: Handle NimBLE transmission
void ble_task(void* pvParameters) {
  // NimBLE Initialization
  NimBLEDevice::init(NIMBLE_DEVICE_NAME);
  NimBLEDevice::setMTU(512);

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

  const int BATCH_SIZE = 20;
  SensorData batch[BATCH_SIZE];
  int batch_index = 0;

  while (true) {
    // Receive one item from the queue and put it in the batch array
    if (xQueueReceive(sensorQueue, &batch[batch_index], portMAX_DELAY) == pdPASS) {
      batch_index++;
      
      // Once we have collected 20 items, send them all at once
      if (batch_index >= BATCH_SIZE) {
        if (deviceConnected && pSensorChar->getSubscribedCount() > 0) {
          pSensorChar->setValue((uint8_t*)batch, sizeof(batch));
          pSensorChar->notify();
        }
        batch_index = 0; // Reset index for the next batch
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) { delay(10); }

  Serial.println("\n--- Starting Sinusoid Generator + NimBLE ---");
  
  // Increase queue size to hold 3 full batches (60 items) to prevent dropping
  sensorQueue = xQueueCreate(60, sizeof(SensorData));
  if (sensorQueue == NULL) {
    Serial.println("Failed to create Sensor Queue!");
    while(1) delay(100);
  }

  // Pin BLE to Core 0, and Sensor Generation to Core 1
  xTaskCreatePinnedToCore(ble_task, "BLETask", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(sensor_task, "SensorTask", 4096, NULL, 2, NULL, 1);
}

void loop() {
  vTaskDelete(NULL);
}
