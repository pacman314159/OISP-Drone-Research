# Original User Request

## 2026-09-17T11:55:51Z

Implement BLE transmission for `firmware/FC_v01` based on `firmware/ble_9dof_daq`, refactored into the 5-layer RTOS architecture with Zettelkasten rules and future BLE OTA support.

Working directory: d:\OneDrive_MSFT\Work\Drone_Research\firmware\FC_v01
Integrity mode: development

## Requirements

### R1. BLE Telemetry Module & Gyro Transmission
Port the NimBLE transmission logic from `ble_9dof_daq` into `FC_v01` as a decoupled Layer 2/3 driver and telemetry module. Configure NimBLE device name `"FC v01"`, MTU (512 bytes), and custom telemetry characteristic notifications for 3-axis Gyroscope (`gx`, `gy`, `gz`) sample batches.

### R2. 5-Layer Decoupled RTOS Architecture & Centralized IPC
Enforce strict compliance with Zettelkasten decisions (`0000`-`0004`):
- Declare `enum TaskCores` in `include/config.h` and assign `accel_gyro_daq_task` to Core 1 (`TASK_CORE_APP`) and `ble_telemetry_task` to Core 0 (`TASK_CORE_PRO`).
- Centralize task priority (`TASK_PRIORITY_LOW`), stack size (8192 B), loop frequency, UUIDs, and core assignments in `include/config.h`.
- Centralize thread-safe queues (`gyro_telemetry_queue`) and server callback structures inside Layer 3 (`src/middleware/ipc.h` / `ipc.cpp`).

### R3. Modular GATT Service Layout & OTA Partition Table
Structure the BLE GATT server layout into distinct telemetry and reserved OTA services (OTA Service UUID, Control Characteristic, Data Characteristic). Update `platformio.ini` to use a custom partition scheme (`partitions.csv`) featuring dual OTA app partitions (`ota_0`, `ota_1`) and `otadata` to support future OTA firmware updates.

## Acceptance Criteria

### Task & IPC Integrity
- [ ] Task core assignments (`enum TaskCores`), priorities, stack sizes, names, and BLE UUIDs are declared centrally in `include/config.h`.
- [ ] BLE task receives 3-axis gyro data safely from `accel_gyro_daq_task` via thread-safe IPC queue (`gyro_telemetry_queue`) in `ipc.h`.
- [ ] Core 1 500 Hz IMU loop timing is completely unaffected by BLE connection state or notifications.
- [ ] Telemetry characteristic notifies 3-axis Gyroscope sample batches.

### Code Style & Formatting Compliance
- [ ] Single-line control flow bracing omitted (`if(cond) return;`).
- [ ] No space between closing parenthesis and opening brace for functions (`void func(){`) and control keywords (`if(cond){`, `}else{`).
- [ ] No `auto` variable declarations used.

### Build & Flash Compatibility
- [ ] PlatformIO configuration (`platformio.ini`) includes `NimBLE-Arduino` library dependency and custom partition table `partitions.csv`.
- [ ] Build configuration compiles cleanly without header duplication or magic numbers.

## 2026-09-17T11:58:11Z

Requirement update from user:
The `TaskCores` enum in `include/config.h` must support unpinned/both cores execution via `TASK_CORE_BOTH = tskNO_AFFINITY` (in addition to `TASK_CORE_PRO = 0` and `TASK_CORE_APP = 1`).
For now, both `accel_gyro_daq_task` (`TASK_IMU_DAQ_CORE`) and `ble_telemetry_task` (`TASK_BLE_CORE`) must be set to `TASK_CORE_BOTH`. Please update your implementation tasks accordingly.

## 2026-09-17T12:01:47Z

Correction on TaskCores enum layout from user:
The `TaskCores` enum in `include/config.h` must follow the exact same pattern as `TaskPriorities`, `TaskNames`, `TaskSizes`, and `TaskFrequencies`, containing each task's core directly as enum members:

```cpp
enum TaskCores : int32_t {
  TASK_IMU_DAQ_CORE = tskNO_AFFINITY,
  TASK_ATTITUDE_CORE= 1,
  TASK_BARO_DAQ_CORE= 1,
  TASK_MAG_DAQ_CORE = 1,
  TASK_BLE_CORE     = tskNO_AFFINITY
};
```

Please adjust your implementation to use this exact `enum TaskCores` structure in `include/config.h`.
