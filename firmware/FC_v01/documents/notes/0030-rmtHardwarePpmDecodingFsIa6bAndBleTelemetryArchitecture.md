---
id: "0030"
title: ESP32-S3 Hardware RMT PPM Receiver Decoding (Flysky FS-iA6B), Layer 1-4 Decoupled Architecture & BLE Real-Time Telemetry Pipeline
author: Drone Research Team / Developer
date: 2026-10-15
time: 19:40:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#rc_rx", "#ppm", "#rmt", "#flysky", "#fs_ia6b", "#ble", "#telemetry", "#fsm"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
  - "[[0009-distinctBleGattCharacteristicUuidIsolation]]"
  - "[[0010-bleTelemetryMinPacketFrequencyAndDualTriggerFlushes]]"
  - "[[0011-variableBleTelemetryPacketParsingAndZeroHandshakeDesign]]"
  - "[[0012-ws2812RgbLedExternalHardwareIsolationAndCentralizedMutexEncapsulationRule]]"
  - "[[0016-layer4CoreDirectoryCompactionAndSubmoduleConsolidation]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0021-bleTelemetryDispatcherConsolidationAndSubscriptionGating]]"
  - "[[0027-preflightSensorDataStreamCheckFsmArchitecture]]"
  - "[[0029-unificationOfInterTaskRingBufferSizeConstant]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
  - "[[0032-rcReceiverCommandBlackboxSdCardLoggingArchitecture]]"
---

# Zettel 0030: ESP32-S3 Hardware RMT PPM Receiver Decoding (Flysky FS-iA6B), Layer 1-4 Decoupled Architecture & BLE Real-Time Telemetry Pipeline

## 1. Metadata & Scope
- **ID**: `0030`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-15
- **Time**: 19:40:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0002]`, `[Note 0006]`, `[Note 0009]`, `[Note 0010]`, `[Note 0011]`, `[Note 0012]`, `[Note 0016]`, `[Note 0020]`, `[Note 0021]`, `[Note 0027]`, `[Note 0029]`

---

## 2. Problem Statement & Architectural Rationale

Quadrotor flight control requires continuous, jitter-free reception of pilot command channels (Roll, Pitch, Yaw, Throttle, Aux1, Aux2) from the Radio Control (RC) receiver. For the **Flysky FS-iA6B** receiver operating in **PPM (Pulse Position Modulation)** mode:
1. **CPU Interrupt Starvation Threat**: Standard software GPIO edge interrupts (`attachInterrupt()`) fire on every microsecond transition (14 to 18 edges per 20 ms frame). Under high-rate 500 Hz flight loop execution and active FreeRTOS scheduling, software ISR latency introduces timing jitter in pulse width measurement and steals critical CPU cycles from the cascaded PID attitude control loops.
2. **Hardware Offloading Mandate**: As established in `[Note 0001]`, PPM pulse timing must be delegated completely to the **ESP32-S3 RMT (Remote Control Transceiver)** peripheral. The RMT hardware counts clock cycles independently into dedicated hardware RAM, filtering noise and asserting an interrupt only when a full frame boundary is detected.
3. **Peripheral Resource Conflict Prevention & ESP32-S3 Hardware RMT Architecture**: `[Note 0012]` assigns RMT Channel 1 (TX) to drive WS2812 status LEDs. On the **ESP32-S3**, RMT hardware channels are strictly unidirectional: Channels 0–3 are dedicated TX-only, while **Channels 4–7 are dedicated RX-only**. Therefore, the RC receiver driver must strictly utilize **RMT Channel 4 (RX)** to match ESP32-S3 silicon architecture and avoid silent reception failure.
4. **Live Telemetry & FCDatMon Integration**: Ground station monitoring and bench validation require streaming decoded RC channels over BLE in real time without interfering with sensor telemetry streams.

---

## 3. Detailed Multi-Layer Architecture

The implementation strictly adheres to the 5-layer firmware hierarchy:

```
+--------------------------------------------------------------------------+
| Layer 5: App Domain (`src/app/app.cpp`, `src/app/fsm.cpp`)                |
|          - Spawns `rc_receiver_task`                                     |
|          - Pre-flight RC link validation (`check_rc_controller_signal`)   |
+--------------------------------------------------------------------------+
| Layer 4: Flight Core Domain (`src/core/interface/`, `src/core/telemetry/`)|
|          - `rc_receiver_task` (50 Hz DAQ loop, failsafe event signaling)  |
|          - `ble_telem_task` (`s_ch_rc` 28-byte streaming channel)         |
+--------------------------------------------------------------------------+
| Layer 3: Middleware IPC (`src/middleware/ipc.h/.cpp`)                    |
|          - `RingBuffer<RcChannels, RING_BUF_SIZE> rc_data`               |
|          - `RingBuffer<uint32_t, RING_BUF_SIZE> rc_timestamp`           |
+--------------------------------------------------------------------------+
| Layer 2: Drivers Domain (`src/drivers/rc_rx/fs_ia6b.h/.cpp`)              |
|          - Driver `FS_iA6B` parsing `rmt_item32_t` duration pulses        |
|          - 800 - 2200 µs pulse validation, 250 ms failsafe timeout       |
+--------------------------------------------------------------------------+
| Layer 1: Hardware Abstraction & Platforms (`src/platforms/`)             |
|          - Pure HAL: `HAL_PulseRX` (`platforms/hal/hal_pulse.h`)          |
|          - Target: `ESP32_PulseRX` (`platforms/targets/espressif/`)      |
|          - Hardware RMT Channel 0 RX (1 µs resolution, glitch filter)    |
+--------------------------------------------------------------------------+
```

### A. Layer 1: Hardware Pulse RX Abstraction (`HAL_PulseRX` & `ESP32_PulseRX`)
- **HAL Contract** (`src/platforms/hal/hal_pulse.h`):
  ```cpp
  class HAL_PulseRX {
  public:
    virtual ~HAL_PulseRX() = default;
    virtual bool init(uint8_t pin, uint32_t glitch_filter_us, uint32_t idle_threshold_us) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual size_t receive_pulses(uint32_t* pulse_buffer, size_t max_pulses, uint32_t timeout_ms) = 0;
  };
  ```
- **ESP32-S3 RMT RX Implementation** (`src/platforms/targets/espressif/pulse_rmt.h/.cpp`):
  - Hardware peripheral: `RMT_CHANNEL_4` (ESP32-S3 dedicated RX channel range 4–7).
  - Clock source: APB Clock (80 MHz) with `clk_div = 80`, giving exact $1.0\,\mu\text{s}$ tick resolution.
  - Glitch filter threshold: $100\,\mu\text{s}$ (hardware automatically rejects RF spikes shorter than $100\,\mu\text{s}$).
  - Idle threshold: $3000\,\mu\text{s}$ ($3.0\,\text{ms}$ low level triggers RMT hardware frame end interrupt, segmenting consecutive PPM frames).
  - Ring buffer: 1024-byte kernel ring buffer backing RMT interrupt transactions.

### B. Layer 2: Flysky FS-iA6B Receiver Driver (`FS_iA6B`)
- In PPM modulation, each channel is encoded as a pulse interval between successive falling/rising edges:
  $$T_{\text{pulse}} = \text{duration0} + \text{duration1} \quad (1000\,\mu\text{s} \le T_{\text{pulse}} \le 2000\,\mu\text{s})$$
- Each `rmt_item32_t` returned by the hardware holds a duration pair. The driver accumulates these into channel slot arrays (`ch.channels[i]`).
- **Validity Guardrails**:
  - Pulse width must fall within $[800\,\mu\text{s}, 2200\,\mu\text{s}]$. Values outside are treated as corrupt frame artifacts.
  - Channels decoded: up to 8 channels (FS-iA6B outputs 6 standard channels: Roll, Pitch, Throttle, Yaw, Ch5/Aux1, Ch6/Aux2).
  - Failsafe timeout: $250\,\text{ms}$. If no valid frame is received within 250 ms, `channels.failsafe` is asserted true and throttle defaults to $1000\,\mu\text{s}$.

### C. Layer 3: Middleware IPC Buffering
- Consistent with `[Note 0029]`, inter-task communication uses unified ring buffer sizing:
  ```cpp
  // src/middleware/ipc.h
  extern RingBuffer<RcChannels, RING_BUF_SIZE> rc_data;
  extern RingBuffer<uint32_t, RING_BUF_SIZE>   rc_timestamp;
  ```

### D. Layer 4: Receiver Task & FSM Coordination
- **Task Runner** (`src/core/telemetry/rc_receiver_task.cpp`):
  - Frequency: 50 Hz (`TASK_RC_RECEIVER_FREQ_HZ = 50`), matching receiver frame cadence (~20 ms).
  - Anti-windup overrun guard on `vTaskDelayUntil`.
  - SystemView markers: `SYSVIEW_START(TASK_RC_RECEIVER_ID)` and `SYSVIEW_END(TASK_RC_RECEIVER_ID)`.
  - Event signaling:
    - Normal reception: `fsm_set_event(EVT_RC_SIGNAL_OK); fsm_clear_event(EVT_RC_SIGNAL_FAIL);`
    - Failsafe: `fsm_set_event(EVT_RC_SIGNAL_FAIL); fsm_clear_event(EVT_RC_SIGNAL_OK);`

---

## 4. BLE Telemetry Stream Specification for FCDatMon

To visualize live transmitter stick movements on the **FCDatMon** UI:

### A. Dedicated GATT Characteristic UUID
Conforming to `[Note 0009]`, a unique UUID is assigned:
```cpp
// include/config.h
constexpr const char* BLE_CHAR_RC_CHANNELS_UUID = "5b8c1a2e-4f9a-4c72-9e3d-8b1a2c3d4e5f";
constexpr uint16_t BLE_TELEM_MASK_RC_CHANNELS  = (1 << 11);
```

### B. 28-Byte Binary Packet Structure (`BleRcPacket`)
The binary layout matches FCDatMon's expected stride:
$$S = 4 + 6 \times 4 = 28\,\text{bytes}$$
```cpp
// src/drivers/telemetry/ble_packet.h
#pragma pack(push, 1)
struct BleRcPacket {
  uint32_t timestamp_us;     // Flight controller microsecond timestamp
  float    channels[6];      // Ch 1-6 pulse width in microseconds (1000.0 - 2000.0)
};
#pragma pack(pop)
static_assert(sizeof(BleRcPacket) == 28, "BleRcPacket layout must be strictly 28 bytes packed");
```

### C. Packet Capacity & Dual-Trigger Flushing
- **Capacity**:
  $$N_{\text{samples}} = \left\lfloor \frac{512 - 3}{28} \right\rfloor = 18 \quad \implies \text{Configured to } 17 \text{ samples} \quad (17 \times 28 = 476\,\text{bytes} \le 512\,\text{MTU})$$
- **Flush Policy** (`[Note 0010]`):
  - Primary trigger: buffer fills to 17 samples.
  - Secondary timeout trigger: $100\,\text{ms}$ elapsed ($10\,\text{Hz}$ minimum telemetry floor), preventing packet stall when sample rate is low.

---

## 5. Pre-Flight Safety & Transition Gating

In `src/app/fsm.cpp` (`check_rc_controller_signal`, Gate 6):
- Inspects `rc_data`:
  - If failsafe flag is active (`ch.failsafe == true`), asserts `EVT_RC_SIGNAL_FAIL` and clears `EVT_RC_SIGNAL_OK`, triggering immediate one-way transition to **`STATE_DEVELOPMENT`** (`[Note 0031]`).
  - If healthy frames arrive within `SENSOR_STREAM_STARTUP_TIMEOUT_MS` (2.5 s), asserts `EVT_RC_SIGNAL_OK`.
  - If no RC frames arrive within the 2.5 s startup timeout window, asserts `EVT_RC_SIGNAL_FAIL`, locking the system in `STATE_DEVELOPMENT` (zero throttle, amber LED).

---

## 6. Traceability & Cross-References
- [[0001-taskConfigMiddlewareIsrDecision]]: RMT PPM decoding and centralized task configurations.
- [[0009-distinctBleGattCharacteristicUuidIsolation]]: Dedicated GATT characteristic for RC channels.
- [[0010-bleTelemetryMinPacketFrequencyAndDualTriggerFlushes]]: Dual-trigger flush mechanism for BLE telemetry.
- [[0012-ws2812RgbLedExternalHardwareIsolationAndCentralizedMutexEncapsulationRule]]: RMT Channel 1 reservation for WS2812 LED.
- [[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]: FSM pre-flight gating and events.
- [[0029-unificationOfInterTaskRingBufferSizeConstant]]: Unified ring buffer capacity sizing.
