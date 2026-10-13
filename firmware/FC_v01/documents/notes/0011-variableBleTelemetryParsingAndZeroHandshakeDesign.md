---
id: "0011"
title: Variable BLE Telemetry Packet Parsing and Zero-Handshake Design
author: Drone Research Team / Developer
date: 2026-10-02
time: 01:07:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#fcdatmon", "#ble", "#telemetry"]
references:
  - "[[0009-distinctBleGattCharacteristicUuidIsolation]]"
  - "[[0010-bleMinimumPacketFrequencyAndDualTriggerFlush]]"
---

# Zettel 0011: Variable BLE Telemetry Packet Parsing and Zero-Handshake Design

## 1. Prime Directive: Fast, Simple & Real-Time
The foundational criterion for FCDatMon is **instant plug-and-play simplicity, zero-friction connection, and true real-time plotting**. 

## 2. Permanent Rejection of Pre-Connection Handshakes
Pre-transmission negotiation (handshaking packet lengths or sensor schemas) is **strictly forbidden**:
- **Latency Compromise**: Negotiating fixed packet sizes forces either lagging slow sensors (Mag takes 400ms to fill 31 samples) or radio-saturating fast sensors (Gyro at 500Hz flooding 63 packets/sec).
- **Complexity & Brittleness**: Handshakes introduce multi-step state machines that stall on packet loss or reconnects. Connection must remain 100% stateless.

## 3. Dynamic User-Configured Sample Stride Architecture
Firmware transmits variable-length datagrams bounded by [Zettel 0010] (capacity trigger of 31 samples OR 100ms countdown timer flush). FCDatMon derives sample count dynamically without in-band headers:

1. **User-Determined Stride ($S$)**: Sample size is never hardcoded. It is calculated directly from the total series lines configured by the user across active plots:
   $$S = (4\text{ if TX else } 0) + (N_{\text{series}} \times 4)\text{ bytes}$$
2. **Datagram Sample Derivation**:
   $$N_{\text{samples}} = \frac{\text{len}(\text{data})}{S}$$
3. **Integrity Validation**: Discard only if $\text{len}(\text{data}) \pmod S \ne 0$.

This natively supports both full 31-sample bursts (500Hz IMU) and partial 7–10 sample timeout flushes (75Hz Mag) with zero configuration friction.
