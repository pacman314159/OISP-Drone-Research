# Poster Draft — HCMUT 2026 (shared with PKNU)

> Status: SKELETON v2 (2026-11-20, EN). Every `[NUM: ...]` is a placeholder —
> fill after Step 0 (both envs build green) and the SystemView re-profile.
> Shared rule: no sentence may imply the vehicle has flown; every number must
> trace back to the receipt table in §R.

---

## Title (candidates)

1. "A Measured Five-Layer RTOS Architecture for Deterministic Flight Control on ESP32-S3"
2. "FC-V01: A Verifiable Software Architecture for Hard Real-Time Flight Control on a Low-Cost MCU"
3. "Architecture as Contribution: Layering Rules, Measured Determinism, and Audit Discipline for an ESP32-S3 Flight Controller"

---

## ① INTRODUCTION (~80–100 words)

- **Context:** A flight controller demands hard real-time behaviour: DAQ →
  preprocessing → state estimation → control must execute on exact periods;
  one late step corrupts the whole chain.
- **Gap (verified, see §R):** In the PX4 ecosystem ESP32 appears only as a WiFi
  telemetry module; the PX4 autopilot catalogue contains no ESP32 board. The
  ArduPilot ESP32/S3 port exists but is officially "experimental" and no OEM
  produces an ESP32-based autopilot. Espressif's ESP-Drone proves feasibility
  but has had limited support since 12/2022. Papers and DIY projects prove
  feasibility (see §RW), yet no platform publishes a system-level determinism
  measurement together with a verifiable architecture ruleset.
- **Claim:** Feasibility is already demonstrated by the community; the remaining
  gap is **measured determinism + architectural rules** — the contribution of
  this work.

## ② METHODOLOGY (largest block — ★ diagram)

### 2.1. Five-layer architecture (★ centre diagram, ~40% of block)
- L5 Application (`src/app/`, FSM) → L4 Flight Core & Algorithms (`src/core/`)
  → L3 Middleware & IPC (`src/middleware/`) → L2 Drivers (`src/drivers/`) →
  L1 Platform HAL (`src/platforms/`).
- Rule: dependencies point **downward only** (strict downward DAG) — lower
  layers never import higher layers.
- **L1 compute HAL (SIMD — one instruction, multiple data):** `hal_simd` exposes
  vector primitives (`dot`, `add`, `sub`, `mul`, `scale`, `mat_mul`, `inv3x3`)
  for `float`, packed `int16` and packed `int8`. On ESP32-S3 these map 1:1 to
  Espressif's **ESP-DSP** routines (`dsps_*`, `dspm_*`), which use the S3's
  **PIE SIMD instruction set**. `hal_matrix` is the type-safe matrix facade
  over that engine.
  - **Why it matters:** math-heavy L2–L4 code (EKF, calibration, filters) stays
    target-agnostic — no vendor header leaks outside L1 — while still getting
    hardware SIMD throughput.
- No target-specific type leaks into any L2–L4 public header.

### 2.2. Experimental setup (formal)

**Hardware platform (self-designed).** The flight-controller carrier is the
authors' own design: a 4-layer KiCad PCB (FC_v01; an FC_v02 revision is in
progress) plus a companion power-monitor board, hosting an ESP32-S3 module,
the GY-87 sensor module (MPU6050 IMU + HMC5883L magnetometer + BMP180
barometer), a MicroSD blackbox socket, WS2812B status LEDs and a LoRa
footprint. Gerbers and BOM are released with the source repository.

**Instrumentation.** RTOS-level behaviour is recorded with a SEGGER
SystemView trace covering the critical-flight task set — sensor DAQ (500 Hz),
sensor preprocessing (500 Hz), attitude EKF (500 Hz), yaw EKF (100 Hz),
altitude EKF (100 Hz), FSM (100 Hz), motor loop (250 Hz), SD logging
(100 Hz), BLE telemetry (100 Hz), RC receiver (50 Hz). The BLE link cannot
carry an RTOS trace (bandwidth + reliability constraints make it infeasible);
the trace is captured over the JTAG app-trace channel (SEGGER SystemView via
OpenOCD) during bench sessions — the mechanism already configured in-tree.
Sensor measurements and
estimator outputs (attitude, altitude, vertical velocity) export via SD or
BLE.

**Protocol.** Bench validation, tethered/clamped — free flight not yet
performed and the control law (INDI) is future work. Each configuration is
repeated over N runs; per-task execution time and scheduling **jitter** are
extracted from the trace and reported as median / p99 / max under
**full-system load with RF active**. This dataset is mandatory for every
determinism claim on this poster.
[NUM: pending Step 0 build + re-profiling — dataset not yet available]

**Calibration.** Sensor inputs are corrected by measured error models:
MPU6050 gyro by static offset; accelerometer by **6-point** ellipsoid fit
(axis-aligned misalignment model — 6 degrees of freedom: 3 scale + 3 offset);
magnetometer by **9-point** ellipsoid fit (rotated ellipsoid — 9 DOF: 3 scale
+ 3 offset + 3 orientation). Both fits are SIMD-accelerated with zero dynamic
allocation. *De-prioritised on the poster: full calibration methodology moves
to the paper text.*

**Instrumentation note.** The CPU clock is set to 240 MHz in both
`sdkconfig.fc-v01` and `sdkconfig.fc-v01-ota` (2026-11-20; previously 160 MHz,
which contradicted note 0007's assumption). Confirm the effective frequency
after the build with `esp_clk_cpu_freq()`. Trace capture uses the JTAG
app-trace channel — the bottleneck-free route on the bench.

**Capability & schedulability highlight.** Static analysis of the current
tree (14 concurrently resident tasks — calibration and OTA tasks spawn on
demand) yields a theoretical average load of ≈20–22% of the dual-core budget
with BLE telemetry (100 Hz) active. The open question — and the central
measured claim of this poster — is the **deadline margin of the 500 Hz
chain**: the measured DAQ WCET (1,678 µs including blocking, note 0007) is
≈84% of the 2 ms budget at the 240 MHz core clock — a margin that must be
re-confirmed under full-system load, not assumed.
[NUM: measured WCET + jitter distribution over ≥10⁴ loops — pending Step 0]

**Verification layer.** Architectural invariants stay checkable while the
experiments run: decision records (Zettel 0000–0052) and gated audits
(ATAM-style evaluation) keep every rule traceable.

## ③ RESULTS (one number per item + source)

> Every number below awaits filling — source in §R. The existing SystemView
> data (note 0007) is from an early DAQ-only phase: use only with its scope
> stated explicitly.

- **System frequency table** (from `config.h`; static source — usable now):
  IMU DAQ 500 Hz / preprocess 500 Hz / EKF roll-pitch 500 Hz / EKF yaw 100 Hz /
  EKF altitude 100 Hz / FSM 100 Hz / baro 50 Hz / mag 100 Hz / RC 50 Hz /
  motor 250 Hz / SD log + BLE telem 100 Hz. DAQ loop budget = 2.0 ms.
- **Dual-core ESP32-S3 task map:** [NUM: SystemView full-system + RF load —
  re-measure after Step 0; old 0007 data = DAQ-only phase]
- **EKF dual-task async:** 7-state quaternion + gyro bias (note 0032);
  roll/pitch 500 Hz, yaw 100 Hz with unit-sphere mag fusion; HCMC magnetic dip
  +9.78°. [NUM: per-task timing/accuracy]
- **Altitude EKF 3-state (baro + accel):** [NUM: 24.15% — OFFLINE REPLAY ONLY,
  note 0037; state "offline replay" on the poster]
- **SIMD calibration (zero dynamic RAM):** 3D ellipsoid mag calibration +
  6-point accel, vectorised through the L1 compute HAL (ESP-DSP / S3 PIE).
  [NUM: calibration result from a dated log]
- **Five-gate preflight pipeline** (note 0052, after the INA226 removal).
- **End-to-end log/telemetry:** SD blackbox 100 Hz + BLE telemetry + BLE OTA.
- **Honesty box (mandatory):** "Measured on bench + static thrust; free flight
  not yet performed."

## ④ DISCUSSION

- The architecture rules work as verifiable commitments: the audit caught a
  cycle (C3), the matrix move required zero call-site changes, and the 3
  remaining layering exceptions each have a measured cost and a closure plan
  (C15). → the architecture is not a drawing; it is an enforced invariant.
- Trade-off: the centralised IPC hub — 150+ direct touches from outside L3
  (re-counted 2026-11-19) is a deliberate, measured debt being reduced item by
  item through the audit.
- Honest limitations: (i) existing SystemView data is DAQ-only, no full-system +
  RF load yet; (ii) 24.15% is offline replay; (iii) the audited tree has not
  been recompiled — no certified binary; (iv) no free flight yet.
- Position versus related work (see §RW): no platform publishes a full-system
  determinism trace; the closest work (arXiv 2607.23220) claims "jitter-free"
  on Arduino-compatible code without a profiler trace.

## ⑤ CONCLUSION & CONTRIBUTIONS

Three contributions:
1. A five-layer RTOS architecture for an ESP32-S3 FC with **measured**
   determinism (SystemView), 500 Hz DAQ + EKF.
2. Zero-dynamic-allocation design + zero-RAM SIMD calibration — a technique set
   for RAM-constrained MCUs.
3. An evidence-driven process: decision records + gated architecture audits — a
   reusable model for embedded projects.
4. Self-designed flight-controller hardware: 4-layer carrier PCB (FC_v01 →
   FC_v02) + companion power-monitor board, gerbers and BOM released.

Future work: INDI controller (slot 0034), mixer, free flight, full-system +
RF-load re-profiling.

## REFERENCES (filtered by tier)

1. PX4 Dev Team, "ESP32 WiFi Module", PX4 Guide (main), docs.px4.io/main/en/telemetry/esp32_wifi_module
2. PX4 Dev Team, "WiFi Telemetry Radio", docs.px4.io/main/en/telemetry/telemetry_wifi
3. PX4 Dev Team, "Flight Controller (Autopilot) Hardware", docs.px4.io/main/en/flight_controller/
4. ArduPilot Dev Team, "ESP32 Based Autopilot", ardupilot.org/dev/docs/esp32-autopilot.html
5. Espressif, "ESP-Drone", github.com/espressif/esp-drone (GPL-3.0; Crazyflie-derived)
6. Mahbub, Roy, Prince, "Embedded Firmware Development for Flight Control, Telemetry, and Video Streaming for DIY UAV Research", arXiv:2607.23220, 2026
7. Parikh, Vohra, Kaveshgar, "Comparison of Attitude Estimation Algorithms With IMU Under External Acceleration", IEEE iSES 2021
8. Moussaoui et al., "Experimental study of PID for attitude control of a quadcopter using an ESP32", IJPEDS 15(3), 2024
9. da Silva, Fazzolari, Correa, "Design and Implementation of a Real Time Attitude Estimation System with Low Cost Sensors", SBA/CBA (UFABC)
10. FreeRTOS (Kernel + SMP), POSA vol.1 (Layers), Clean Architecture — methodology references

---

## §R. RECEIPT TABLE — "numbers/citations cleared for use"

| # | Claim | Source | Verified | Status |
|---|---|---|---|---|
| R1 | ESP32 appears only under Telemetry Wifi (DroneBridge) in PX4 docs | docs.px4.io/main/en/telemetry/esp32_wifi_module | 2026-11-20 | verified |
| R2 | The PX4 autopilot catalogue contains no ESP32 board (STM32/H7 + RPi, BeagleBone experimental) | docs.px4.io/main/en/flight_controller/ | 2026-11-20 | verified |
| R3 | ESP32 is listed among "Compatible WiFi Telemetry modules" | docs.px4.io/main/en/telemetry/telemetry_wifi | 2026-11-20 | verified |
| R4 | ArduPilot ESP32 port: "experimental", "No OEM has produced...", ESP32S3 work in progress | ardupilot.org/dev/docs/esp32-autopilot.html | 2026-11-20 | verified |
| R5 | S3 support is being added incrementally (board esp32s3m5stampfly) | github.com/ArduPilot/ardupilot/pull/29448 (merged 2025-03) | 2026-11-20 | verified |
| R6 | Community: ESP32 loop rate lower than STM FCs (rover 50 Hz vs 400 Hz) — community anecdote | discuss.ardupilot.org ESP32-S3 threads 2025-12→2026-06 | 2026-11-20 | [unconfirmed — anecdote, use with caveat] |
| R7 | ESP-Drone: Crazyflie-derived, limited support since 12/2022 | github.com/espressif/esp-drone README | 2026-11-20 | verified |
| R8 | 19-task frequency table (config.h) | firmware/FC_v01/include/config.h:202–221 | 2026-11-20 | verified (static source) |
| R9 | 155 cross-layer includes, 3 known upward edges (C5 + 2 C3 sizeof remains) | automated scan of main tree + audit report | 2026-11-19/20 | verified |
| R10 | SystemView 0007 data = early DAQ-only phase | note 0007 + state.md | — | [experimental — use only with stated scope] |
| R11 | 24.15% altitude = offline replay | note 0037 | — | [experimental — state "offline replay"] |
| R12 | No work in §RW publishes a full-system determinism trace | research session 2026-11-19 | 2026-11-19/20 | verified (negative claim, within the searched set) |

## §RW. RELATED WORK — positioning

| Source | Platform | Closest to us | How we differ |
|---|---|---|---|
| arXiv 2607.23220 (2026) | ESP32-S3 dual-core, 400 Hz ctrl + telemetry/video | Core separation, zero-copy | FreeRTOS 19 task + SystemView trace + layering ruleset + 500 Hz |
| IEEE iSES 2021 | ESP32, benchmark of 4 attitude filters | Timing benchmark on the same chip | 7-state EKF + 2-task async + architectural rules |
| IJPEDS 2024 | ESP32, PID, actually flown | Flight-proven | No determinism measurement |
| UFABC (CBA) | ESP32 + MPU-9250, float 1.70 ms / double 4.29 ms | Comparable timing | Full-system SystemView measurement |
| IJRTI 2026 | ESP32, "deterministic 250 Hz" | Determinism claim | 500 Hz + EKF, not complementary-only |
| IJRASET 2026 | ESP32-S3 micro drone | Same S3 + MPU6050 | Low-tier — existence proof only |
| Belgrade 2025 | ESP32-S3 Zero, MIMO PID, "controlled flight was not achieved" | Honest about difficulty | Supports the "hard problem" narrative |
| esp-drone (Espressif) | ESP32/S2/S3, Crazyflie-derived | Official existence proof | Limited support; no audit/measurement |

**Tier note:** primary citations = PX4 docs, ArduPilot docs, Espressif, arXiv,
IEEE, IJPEDS, SBA/CBA. Low-tier (IJRASET/IJRTI/IJSREM) only as existence proofs.

## NUMBERS TO MEASURE AFTER STEP 0 (fill-in checklist)

- [ ] Both envs build green: `pio run -e fc-v01` + `fc-v01-ota` + POSTFLIGHT smoke
- [ ] SystemView full-system profile + RF load (replaces R10)
- [ ] Per-task timing: DAQ / preprocess / EKF roll-pitch / EKF yaw / FSM
- [ ] Altitude EKF: measure on new data, settle "offline replay" vs realtime
- [ ] SIMD calibration: residuals + fit time
- [ ] 500 Hz loop jitter / worst case (max dt, p99)
