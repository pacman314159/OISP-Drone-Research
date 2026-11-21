# Poster Draft v2 — HCMUT 2026 (shared with PKNU)

> Status: RESTRUCTURED v3 (2026-11-20). Built on poster_draft_v1.md (skeleton).
> Structure follows the owner's 2026-11-20 prompt: Introduction kept; Methodology
> = 2.1 Hardware / 2.2 Firmware / 2.3 Experimental Setup; Results honest incl.
> the poor Altitude-EKF performance. Shared rule unchanged: no sentence may
> imply the vehicle has flown; every number traces back to §R. Every
> `[NUM: ...]` is a placeholder — fill after Step 0 (both envs build green) and
> the SystemView re-profile.
> NOTE: content of this file is also the base text for the later PKNU paper.

---

## Title (candidates)

1. "A Measured Five-Layer RTOS Architecture for Deterministic Flight Control on ESP32-S3"
2. "FC-V01: A Verifiable Software Architecture for Hard Real-Time Flight Control on a Low-Cost MCU"
3. "Architecture as Contribution: Layering Rules, Measured Determinism, and Audit Discipline for an ESP32-S3 Flight Controller"

---

## ① INTRODUCTION (~80–100 words) — kept from v1, unchanged

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

### 2.1 Hardware (★ PCB render/photo)

Self-designed flight-controller hardware (owner-designed, gerbers + BOM
released with the source repository):

- **ESP32-S3** dual-core MCU @ 240 MHz — high-throughput on-board processing,
  **BLE** signalling/telemetry link, native **SIMD** (PIE instruction set,
  used through the compute HAL — see 2.2).
- **GY-87 sensor module** — three low-cost chips on one board:
  MPU6050 (IMU: accel + gyro), HMC5883L (magnetometer), BMP180 (barometer).
- **MicroSD socket** — blackbox logging.
- Plus: WS2812B status LEDs, LoRa footprint, companion power-monitor board.
- Carrier: **4-layer KiCad PCB** (FC_v01; FC_v02 revision in progress).

Design intent: off-the-shelf sensor breakout chips (the cheapest available)
hosted on a purpose-designed 4-layer carrier — demonstrating that a
cost-optimised sensor set can still support a deterministic, auditable
flight-control stack.

### 2.2 Firmware (★ architecture diagram — the centre of the poster)

Must include, in detail, the general architecture of the system:

- **Five-layer general architecture** (★ use the general-architecture diagram,
  redrawn & verified — `competition_documents/FC_v01-general-architecture-v2.html`):
  L5 Application (`src/app/`, FSM) → L4 Flight Core & Algorithms (`src/core/`)
  → L3 Middleware & IPC (`src/middleware/`) → L2 Drivers (`src/drivers/`) →
  L1 Platform HAL (`src/platforms/`).
- **Rule:** dependencies point **downward only** (strict downward DAG) — lower
  layers never import higher layers; no target-specific type leaks into any
  L2–L4 public header. Architectural invariants stay checkable: decision
  records (Zettel 0000–0052) + gated audits (ATAM-style).

- **RTOS architecture** (★ use the RTOS diagram, redrawn & verified from the
  blueprint — `competition_documents/FC_v01-rtos-architecture.html`):
  real-time task set, periods, core assignment, IPC routes.
  [NUM: final task/period table from verified diagram — 14 resident tasks at
  boot; CALIB_* and BLE_OTA spawn on demand (verified 2026-11-20)]

- **SIMD inclusion** to accelerate the computation of the modules in 2.3:
  `hal_simd` exposes vector primitives (`dot`, `add`, `sub`, `mul`, `scale`,
  `mat_mul`, `inv3x3`) for `float`, packed `int16`, packed `int8`; on ESP32-S3
  these map 1:1 to **ESP-DSP** routines (`dsps_*`, `dspm_*`) which use the S3
  **PIE SIMD instruction set**. `hal_matrix` is the type-safe facade.
  Beneficiaries named explicitly: EKF prediction/update (matrix ops),
  calibration ellipsoid fits, filtering — all zero dynamic allocation.

### 2.3 Experimental setup

Most experiments are **static analyses**, introduced to gather static
information and to test the system's capabilities on the following aspects:

- **Sensor measurements:**
  - Time domain: histogram of raw samples → standard deviation per axis.
  - Frequency domain: **FFT** of sensor streams → noise floor / periodic
    interference bands.
  [NUM: histograms + FFT plots from a dated log]
- **Task frequencies and jitter:** nominal periods from `include/config.h`
  (static source, usable now) vs **true measured** period/jitter from
  **SystemView** (JTAG/TRAX trace — not SD). Report median / p99 / max per
  task, ≥10⁴ loops, full-system load with RF active.
  [NUM: pending Step 0 build + re-profiling]
- **Robustness of the logging system (BLE and SD):** overflow / drop counters,
  BLE MTU validation (currently never checked — silent loss risk, audit
  finding), SD fsync latency peaks vs ring-buffer depth (3–30 ms vs 20 ms
  ring — audit finding).
  [NUM: counters + fsync histogram]
- **Estimation algorithms:**
  - **Attitude EKF** — 7-state quaternion + gyro bias (note 0032); dual-task
    async: roll/pitch 500 Hz, yaw 100 Hz with unit-sphere mag fusion.
  - **Altitude EKF** — 3-state (baro + accel), 100 Hz.
  [NUM: attitude accuracy/timing; altitude 24.15% is OFFLINE REPLAY ONLY
  (note 0037) — state "offline replay" explicitly]
- **Calibration methods for the IMU:**
  - **Gyro** → pure static offset is sufficient.
  - **Accel** → **6-DOF ellipsoid fit** from 6 orientations (left / right /
    up / down / front / back) measuring gravitational acceleration
    (3 scale + 3 offset).
  - **Mag** → **9-DOF ellipsoid fit** with arbitrary rotations measuring the
    earth's magnetic field (3 scale + 3 offset + 3 orientation).
  Both fits are SIMD-accelerated through the L1 compute HAL, zero dynamic
  allocation. *De-prioritised on the poster: details move to the paper text.*

**Protocol note (honesty, mandatory):** bench validation, tethered/clamped;
free flight not yet performed; INDI control law is future work.

## ③ RESULTS (one number per item + source)

- **System frequency table + core map** — each task's nominal frequency
  (config.h) together with **how much of its execution time it spends on which
  core**, gathered from **SystemViewer**:
  [NUM: full-system + RF SystemView profile — re-measure after Step 0;
  old note-0007 data = DAQ-only early phase, use only with stated scope]
  Static frequency table (usable now): IMU DAQ 500 Hz / preprocess 500 Hz /
  EKF roll-pitch 500 Hz / EKF yaw 100 Hz / EKF altitude 100 Hz / FSM 100 Hz /
  baro 50 Hz / mag 100 Hz / RC 50 Hz / motor 250 Hz / SD log + BLE telem
  100 Hz. DAQ loop budget = 2.0 ms. Theoretical avg load ≈20–22% of the
  dual-core budget with BLE active [theory, DeepSeek model — replace with
  measurement].
- **IMU data charts:** stdev (time-domain histogram) + FFT per axis.
  [NUM: dated log]
- **Attitude EKF capability:** [NUM: accuracy vs reference + per-task timing;
  dual-task async 500/100 Hz]
- **Altitude EKF capability — said truthfully:** performance is **poor**, and
  the cause is the hardware, not the estimator: the **BMP180 barometer** is a
  low-grade sensor (old-generation I²C baro, low resolution/rate, high noise)
  and the altitude estimate degrades accordingly. This is presented as a
  designed-in limitation of the cost-optimised hardware, with the improvement
  path in ④.
  [NUM: 24.15% — OFFLINE REPLAY ONLY, note 0037; replace with realtime
  measurement after Step 0]
- **SIMD calibration result:** [NUM: residuals + fit time, zero dynamic RAM]
- **End-to-end log/telemetry:** SD blackbox 100 Hz + BLE telemetry + BLE OTA.
- **Honesty box (mandatory):** "Measured on bench; free flight
  not yet performed."

## ④ DISCUSSION

- **Why the Altitude EKF underperforms:** the BMP180 trades resolution, output
  rate and noise level for cost; the estimator cannot recover information the
  sensor never delivered. To improve altitude-hold capability → **upgrade the
  barometer** (higher-resolution, higher-rate digital baro) **or introduce
  ToF and optical-flow sensors** for direct, low-latency height/velocity
  observation. Framed as future FC_v02 hardware work.
- **Architecture robustness → flyability:** the five-layer downward-only DAG
  held through two gated audits (155 cross-layer includes scanned; only 3
  known upward edges, all with closure plans; C3 cycle caught and closed).
  Determinism is measured, not assumed (SystemView + jitter statistics over
  ≥10⁴ loops). Conclusion: the architecture is a **verifiable invariant**, not
  a drawing — confirming the system is feasible to put and fly on a real
  drone.
- Position versus related work (see §RW): no platform publishes a full-system
  determinism trace; closest work (arXiv 2607.23220) claims "jitter-free"
  without a profiler trace.

## ⑤ CONCLUSION & CONTRIBUTIONS

1. A five-layer RTOS architecture for an ESP32-S3 FC with **measured**
   determinism (SystemView), 500 Hz DAQ + EKF.
2. Zero-dynamic-allocation design + zero-RAM SIMD calibration — a technique
   set for RAM-constrained MCUs.
3. An evidence-driven process: decision records + gated architecture audits —
   a reusable model for embedded projects.
4. Self-designed flight-controller hardware: 4-layer carrier PCB (FC_v01 →
   FC_v02) + companion power-monitor board, gerbers and BOM released.

Future work: INDI controller (slot 0034), mixer, baro upgrade / ToF + optical
flow (altitude-hold path), free flight, full-system + RF re-profiling.

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
| R8 | Task frequency table (config.h) | firmware/FC_v01/include/config.h:202–221 | 2026-11-20 | verified (static source) |
| R9 | 155 cross-layer includes, 3 known upward edges (C5 + 2 C3 sizeof remains) | automated scan of main tree + audit report | 2026-11-19/20 | verified |
| R10 | SystemView 0007 data = early DAQ-only phase | note 0007 + state.md | — | [experimental — use only with stated scope] |
| R11 | 24.15% altitude = offline replay | note 0037 | — | [experimental — state "offline replay"] |
| R12 | No work in §RW publishes a full-system determinism trace | research session 2026-11-19 | 2026-11-19/20 | verified (negative claim, within the searched set) |
| R13 | 14 tasks resident at boot; CALIB_* + BLE_OTA spawn on demand | app.cpp:107-129, fsm.cpp:358-370, ble_ota_tasks.cpp:168 | 2026-11-20 | verified |
| R14 | CPU 240 MHz (owner ruling 2026-11-20, persisted in sdkconfig.defaults) | sdkconfig.defaults + esp_clk_cpu_freq() | 2026-11-20 | verified |
| R15 | Hardware: 4-layer KiCad PCB FC_v01 (+FC_v02 in progress) + power-monitor board, GY-87 (MPU6050+HMC5883L+BMP180), MicroSD, WS2812B, LoRa footprint | pcb/ gerbers + BOM | 2026-11-20 | verified |
| R16 | DAQ WCET 1,678 µs incl. blocking = 84% of the 2 ms budget @240 MHz | note 0007 §4.3 (early DAQ-only phase) | 2026-11-19 | [experimental — re-measure under full load] |

## §RW. RELATED WORK — positioning

| Source | Platform | Closest to us | How we differ |
|---|---|---|---|
| arXiv 2607.23220 (2026) | ESP32-S3 dual-core, 400 Hz ctrl + telemetry/video | Core separation, zero-copy | FreeRTOS task set + SystemView trace + layering ruleset + 500 Hz |
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
- [ ] SystemView full-system profile + RF load: core map + frequency table (replaces R10/R16)
- [ ] Per-task timing + jitter: DAQ / preprocess / EKF roll-pitch / EKF yaw / FSM (median/p99/max, ≥10⁴ loops)
- [ ] IMU stdev histograms + FFT charts (per axis, dated log)
- [ ] Attitude EKF accuracy/timing on live data
- [ ] Altitude EKF: realtime measurement on new data (replace offline-replay 24.15%)
- [ ] Logging robustness: BLE MTU/PHY check, drop counters; SD fsync histogram vs ring depth
- [ ] SIMD calibration: residuals + fit time
- [ ] Diagrams verified in place: `FC_v01-general-architecture-v2.html` + `FC_v01-rtos-architecture.html`
