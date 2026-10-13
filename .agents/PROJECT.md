# Project: FC_v01 BLE Telemetry & 5-Layer RTOS Architecture Integration

## Architecture
FC_v01 employs a 5-layer decoupled RTOS architecture (Zettelkasten decisions 0000-0004):
- **Layer 1: Platform HAL** (`src/platforms/`): Hardware abstractions (I2C, SPI, GPIO, SIMD).
- **Layer 2: Drivers** (`src/drivers/`): Sensor drivers (MPU6050, MS5611, QMC5883L) and BLE Driver (`src/drivers/ble/`).
- **Layer 3: Middleware** (`src/middleware/`): Inter-process communication (`ipc.h`/`ipc.cpp`), telemetry streaming (`telemetry/`), filters, ring buffers.
- **Layer 4: Core / Control & Estimators** (`src/control/`, `src/estimators/`): Flight control algorithms, attitude estimation, cascade PID.
- **Layer 5: Application** (`src/app/`, `src/main.cpp`): System lifecycle, task spawning, top-level coordination.

Data Flow for BLE Telemetry:
1. `accel_gyro_daq_task` reads MPU6050 on Core 1 / unpinned at 500 Hz.
2. Ingests raw gyro, scales to `deg/s`, packages into `GyroTelemetrySample` (16 bytes).
3. Pushes into `gyro_telemetry_queue` with `timeout = 0` (strictly non-blocking; zero flight loop jitter).
4. `ble_telemetry_task` on Core 0 / unpinned pops samples from `gyro_telemetry_queue`.
5. Accumulates 15 samples into a batch ($15 \times 16 = 240$ bytes).
6. Dispatches GATT notification via NimBLE characteristic to connected client (MTU 512 negotiated).

## Feature Inventory
| # | Feature | Description | Milestone | Source |
|---|---------|-------------|-----------|--------|
| 1 | Custom Dual OTA Partition Scheme | `partitions.csv` with 8MB flash layout (`ota_0`, `ota_1`, `otadata`, `nvs`, `phy_init`, `storage`) | M1 | ORIGINAL_REQUEST §R3 |
| 2 | PlatformIO & Build Configuration | Update `platformio.ini`, `sdkconfig.defaults`, `CMakeLists.txt` for NimBLE and partition table | M1 | ORIGINAL_REQUEST §R3 |
| 3 | Centralized Task Enums & Constants | Declare `enum TaskCores : int32_t`, `TaskPriorities`, `TaskNames`, `TaskSizes`, `TaskFrequencies`, BLE UUIDs in `config.h` | M1 | ORIGINAL_REQUEST §R2 |
| 4 | Thread-Safe Telemetry Queue IPC | Declare `gyro_telemetry_queue` (depth 60), `GyroTelemetrySample`, connection state in `src/middleware/ipc.h` & `ipc.cpp` | M2 | ORIGINAL_REQUEST §R2 |
| 5 | Non-blocking DAQ Telemetry Push | In `accel_gyro_daq_task`, push gyro samples to `gyro_telemetry_queue` with timeout 0; remove blocking UART `Serial.printf` | M2 | ORIGINAL_REQUEST §R2 |
| 6 | Zero-Heap Static BLE Callbacks | Static `NimBLEServerCallbacks` implementation in Layer 3 middleware to track client connection | M2 | ORIGINAL_REQUEST §R2 |
| 7 | Decoupled Layer 2 BLE Driver | Modular NimBLE driver in `src/drivers/ble/`: init, device name "FC v01", MTU 512, GATT server setup | M3 | ORIGINAL_REQUEST §R1 |
| 8 | Modular GATT Service Architecture | Telemetry Service (Gyro char `READ|NOTIFY`) and Reserved OTA Service (Control `WRITE|NOTIFY`, Data `WRITE|WRITE_NR`) | M3 | ORIGINAL_REQUEST §R3 |
| 9 | Layer 3 BLE Telemetry Task | `ble_telemetry_task` in `src/middleware/telemetry/`: batching 15 gyro samples (240 B), notify client | M3 | ORIGINAL_REQUEST §R1 |
| 10 | System Lifecycle Integration | Wire IPC, initialize BLE driver, spawn `ble_telemetry_task` in `src/app/app.cpp` & `src/main.cpp` | M4 | ORIGINAL_REQUEST §R1, R2 |
| 11 | End-to-End Verification & Gate Approval | Validate 500 Hz timing isolation, zero dynamic allocation, style compliance, and integrity | M4 | Acceptance Criteria |

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| 1 | Build Infra, Partition Scheme & Centralized Config | `partitions.csv`, `platformio.ini`, `sdkconfig.defaults`, `CMakeLists.txt`, `include/config.h` | none | DONE |
| 2 | IPC Middleware Infrastructure & Non-Blocking DAQ Integration | `src/middleware/ipc.h`, `src/middleware/ipc.cpp`, `src/drivers/driver_tasks.cpp` | M1 | IN_PROGRESS |
| 3 | BLE Driver & Telemetry Middleware Implementation | `src/drivers/ble/`, `src/middleware/telemetry/` | M1, M2 | PLANNED |
| 4 | System Integration & Lifecycle Orchestration | `src/app/app.cpp`, `src/main.cpp`, full gate verification | M1, M2, M3 | PLANNED |

## Interface Contracts

### `include/config.h` ↔ System
```cpp
enum TaskCores : int32_t {
  TASK_IMU_DAQ_CORE = tskNO_AFFINITY,
  TASK_ATTITUDE_CORE= 1,
  TASK_BARO_DAQ_CORE= 1,
  TASK_MAG_DAQ_CORE = 1,
  TASK_BLE_CORE     = tskNO_AFFINITY
};

enum TaskPriorities : UBaseType_t {
  TASK_PRIORITY_LOW      = 1,
  TASK_PRIORITY_MED      = 2,
  TASK_PRIORITY_HIGH     = 3,
  TASK_PRIORITY_REALTIME = 4
};

enum TaskSizes : uint32_t {
  TASK_IMU_DAQ_STACK_SIZE   = 4096,
  TASK_ATTITUDE_STACK_SIZE  = 4096,
  TASK_BARO_DAQ_STACK_SIZE  = 2048,
  TASK_MAG_DAQ_STACK_SIZE   = 2048,
  TASK_BLE_STACK_SIZE       = 8192
};

enum TaskFrequencies : uint32_t {
  TASK_IMU_DAQ_FREQ_HZ  = 500,
  TASK_ATTITUDE_FREQ_HZ = 500,
  TASK_BARO_DAQ_FREQ_HZ = 50,
  TASK_MAG_DAQ_FREQ_HZ  = 50,
  TASK_BLE_FREQ_HZ      = 33
};

constexpr const char* BLE_DEVICE_NAME = "FC v01";
constexpr uint16_t BLE_MTU_SIZE = 512;
constexpr uint8_t BLE_TELEMETRY_BATCH_SIZE = 15;

constexpr const char* BLE_SERVICE_TELEMETRY_UUID   = "d3a9f560-8f77-4a45-b3e0-3c22d8f23c91";
constexpr const char* BLE_CHAR_GYRO_TELEMETRY_UUID = "b4f61a92-4c35-4f93-b2e1-89a37f6a5fdd";

constexpr const char* BLE_SERVICE_OTA_UUID      = "23408888-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr const char* BLE_CHAR_OTA_CONTROL_UUID = "23408889-1f40-4cd8-9b89-ca8d45f8a5b0";
constexpr const char* BLE_CHAR_OTA_DATA_UUID    = "2340888a-1f40-4cd8-9b89-ca8d45f8a5b0";
```

### `src/middleware/ipc.h` ↔ DAQ & Telemetry
```cpp
struct GyroTelemetrySample {
  uint32_t timestamp_us;
  float gx; // deg/s
  float gy; // deg/s
  float gz; // deg/s
};

extern QueueHandle_t gyro_telemetry_queue;
bool is_ble_client_connected();
void set_ble_client_connected(bool connected);
NimBLEServerCallbacks* get_ble_server_callbacks();
```

### `src/drivers/ble/` ↔ `src/middleware/telemetry/`
```cpp
bool init_ble_subsystem();
bool notify_gyro_telemetry_batch(const void* data, size_t length);
bool is_gyro_notify_enabled();
```

## Code Layout
- `firmware/FC_v01/partitions.csv`
- `firmware/FC_v01/platformio.ini`
- `firmware/FC_v01/sdkconfig.defaults`
- `firmware/FC_v01/src/CMakeLists.txt`
- `firmware/FC_v01/include/config.h`
- `firmware/FC_v01/src/middleware/ipc.h`
- `firmware/FC_v01/src/middleware/ipc.cpp`
- `firmware/FC_v01/src/drivers/driver_tasks.cpp`
- `firmware/FC_v01/src/drivers/ble/ble_driver.h`
- `firmware/FC_v01/src/drivers/ble/ble_driver.cpp`
- `firmware/FC_v01/src/middleware/telemetry/ble_telemetry.h`
- `firmware/FC_v01/src/middleware/telemetry/ble_telemetry.cpp`
- `firmware/FC_v01/src/app/app.h`
- `firmware/FC_v01/src/app/app.cpp`
- `firmware/FC_v01/src/main.cpp`
