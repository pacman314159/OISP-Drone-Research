---
id: "0007"
title: SEGGER SystemView Trace Instrumentation, OpenOCD Dual-Core Setup & RTOS Loop Profiling Conventions
author: Drone Research Team / Developer
date: 2026-09-25
time: 09:54:00 +07:00
tags: ["#zettelkasten", "#firmware", "#profiling", "#fc_v01", "#sysview", "#openocd", "#freertos", "#jitter", "#wcet"]
references:
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0006-taskCreationConfigEnumsAndTaskArgsPassingDecision]]"
---

# Zettel 0007: SEGGER SystemView Trace Instrumentation, OpenOCD Dual-Core Setup & RTOS Loop Profiling Conventions

## 1. Metadata & Context
- **ID**: `0007`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-09-25
- **Time**: 09:54:00 +07:00
- **Status**: Approved & Enforced
- **Target Hardware**: ESP32-S3 (Xtensa Dual-Core LX7, 240 MHz), On-Chip USB-JTAG (`esp-builtin`)
- **References**: `[Note 0001]`, `[Note 0006]`

---

## 2. PlatformIO & OpenOCD JTAG Tracing Infrastructure

### Unified PlatformIO Environment
To eliminate dual-compilation rebuild delays (saving ~150 seconds per debug cycle), `platformio.ini` is unified under the `[env:esp32-s3-devkitc-1-n8]` profile:
- Flashing & Console: Standard UART on `COM7` (`upload_protocol = esptool`, `monitor_port = COM7`).
- Debugging & Trace: On-chip USB-JTAG interface (`debug_tool = esp-builtin`).

### Kconfig / SDKConfig Prerequisites & Anti-Choking Configuration
The ESP-IDF runtime requires specific tracing buffers and FreeRTOS hooks enabled inside `sdkconfig.defaults` and `sdkconfig.esp32-s3-devkitc-1-n8`. To prevent JTAG buffer saturation, CPU stalls, and corrupted trace files (`Incomplete block`, `Unsupported predef event`), the following settings are strictly enforced:

1. **Zero-Wait Buffer Timeout (Flight Safety Guard)**:
   - `CONFIG_SYSVIEW_BUF_WAIT_TMO=0`
   - `CONFIG_APPTRACE_SV_BUF_WAIT_TMO=0`
   - *Rationale*: Never allow the tracing engine to halt or stall CPU cores when trace buffers fill up. In flight control firmware, dropped trace packets are always preferable to stalling real-time control loops.

2. **Expanded Trace Memory**:
   - `CONFIG_APPTRACE_DEST_JTAG=y`: Routes trace streaming via JTAG memory-mapped ring buffers.
   - `CONFIG_APPTRACE_SV_ENABLE=y`: Enables Segger SystemView trace backend.
   - `CONFIG_SYSVIEW_ENABLE=y`: Activates SystemView API hooks in ESP-IDF.
   - `CONFIG_FREERTOS_USE_TRACE_FACILITY=y`: Enables RTOS task switcher hooks.
   - `CONFIG_APPTRACE_PENDING_DATA_SIZE_MAX=65536`: Expands pending buffer to 64 KB in internal SRAM to absorb multi-core trace bursts.

3. **High-Frequency ISR & Timer Event Suppression**:
   - `CONFIG_SYSVIEW_ENABLE_ISR_ENTER=n`
   - `CONFIG_SYSVIEW_ENABLE_ISR_EXIT=n`
   - `CONFIG_SYSVIEW_ENABLE_ISR_TO_SCHED=n`
   - `CONFIG_SYSVIEW_ENABLE_TIMER_START=n`
   - `CONFIG_SYSVIEW_ENABLE_TIMER_STOP=n`
   - *Rationale*: Suppressing interrupt and timer entry/exit events cuts trace bandwidth by ~75%, retaining only task execution events (`TASK_START_EXEC`, `TASK_STOP_EXEC`, `TASK_START_READY`, `TASK_STOP_READY`, `TASK_CREATE`) and user markers (`SYSVIEW_START`/`SYSVIEW_END`). This prevents USB-JTAG stream choking.

### Dual-Core OpenOCD Trace Capture Syntax & Sizing Sweet Spot
Because the ESP32-S3 is an asymmetric dual-core SoC, OpenOCD requires explicit output paths for both cores simultaneously. 

The command signature in Espressif OpenOCD is:
```text
esp sysview start <out0> <out1> [poll_period_ms] [trace_size_bytes] [stop_timeout_s]
```

> [!WARNING] Parameter Inversion Hazard
> Never pass `-1` as the 3rd argument (`poll_period_ms`). OpenOCD parses unsigned `-1` as `4294967295 ms` (~49.7 days), which completely disables background JTAG buffer polling and halts trace draining. The 3rd parameter must always be `1` (1 ms active polling).

#### Standard Telnet Trace Session:
```bash
# 1. Telnet into OpenOCD (localhost:4444)
telnet localhost 4444

# 2. Maximize JTAG adapter clock speed (20 MHz)
adapter speed 20000

# 3. Capture clean steady-state snapshot (12 KB sweet spot)
esp sysview start file://core0.SVDat file://core1.SVDat 1 12000 -1

# 4. Optional manual stop (if stop_timeout or size limit is not reached)
esp sysview stop
```

---

## 3. Firmware Tracing Middleware (`src/middleware/sysview_tracing.*`)

### Official SEGGER API Mapping
To avoid unresolved symbol compiler errors, the firmware tracing layer wraps official SEGGER / ESP-IDF SystemView APIs:
- User markers: `SEGGER_SYSVIEW_OnUserStart(marker_id)` and `SEGGER_SYSVIEW_OnUserStop(marker_id)`
- Resource registration: `SEGGER_SYSVIEW_NameResource(resource_id, name)`

### Task Runner Instrumentation & Anti-Windup Overrun Guard
In accordance with `[Note 0006]`, tasks in Layer 4 (`src/core/`) access their assigned `TASK_*_ID` from `include/config.h` and wrap the complete loop body.

Additionally, all high-frequency tasks governed by `vTaskDelayUntil()` MUST incorporate an **anti-windup overrun guard** to prevent catch-up bursts when resuming after JTAG halts:

```cpp
void accel_gyro_daq_task(void* arg){
  if(arg == nullptr) return;
  MPU6050* mpu = static_cast<MPU6050*>(arg);
  const uint32_t task_id = TASK_ACCEL_GYRO_DAQ_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_ACCEL_GYRO_DAQ_FREQ_HZ);
  TickType_t last_wake_time = xTaskGetTickCount();

  while(true){
    // Anti-windup overrun guard against JTAG/debugger halt latencies
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)){
      last_wake_time = now;
    }
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id);
    mpu->read_sensor_data();
    // Signal estimators / publish telemetry
    SYSVIEW_END(task_id);
  }
}
```

---

## 4. RTOS Execution Phenomena & Forensic Analysis

### 1. Sub-slice Fragmentation vs. Full Loop
- In FreeRTOS, non-blocking peripheral drivers (e.g. I2C) yield execution to the scheduler while waiting for hardware transfer completion (`I2C_EXT0` interrupt).
- This creates multiple internal execution **sub-slices** per task iteration (e.g., ~$113 \ \mu\text{s}$, ~$95 \ \mu\text{s}$, ~$137 \ \mu\text{s}$).
- Raw SystemView event counts measure every scheduler switch, which yielded misleading sub-slice rates (~$3708 \ \text{Hz}$ / ~$270 \ \mu\text{s}$ interval). The actual flight loop runs deterministically at $500 \ \text{Hz}$ ($2000 \ \mu\text{s}$ period) and must be measured exclusively between `SYSVIEW_START` markers.

### 2. Startup Catch-Up Burst & Trace Buffer Saturation
- If `last_wake_time` falls behind the RTOS tick counter during lengthy hardware initialization, `vTaskDelayUntil()` executes consecutive back-to-back zero-delay loops at ~$812 \ \mu\text{s}$ (~$1231 \ \text{Hz}$) until synchronized.
- This transient burst saturates the $16 \ \text{KB}$ JTAG ring buffer, causing OpenOCD `Incomplete block` warnings.
- **Remedy**: Always initialize `last_wake_time = xTaskGetTickCount()` immediately prior to entering the `while(true)` flight loop, combined with the anti-windup overrun guard.

### 3. Worst-Case Execution Time (WCET) Forensic Breakdown
A forensic analysis of the worst-case `ACCEL_GYRO_DAQ` iteration ($1678 \ \mu\text{s}$ elapsed duration) under contention revealed four distinct execution components:
1. **Mutex Contention (Shared I2C Bus)**: $826 \ \mu\text{s}$ ($49.2\%$). Blocked waiting on `MAG_DAQ` while priority inheritance escalated `MAG_DAQ` priority.
2. **I2C Hardware Wait**: $340 \ \mu\text{s}$ ($20.3\%$). Task blocked in semaphore waiting for the I2C peripheral FIFO interrupt.
3. **Active CPU Execution**: $370 \ \mu\text{s}$ ($22.0\%$). True computation and driver processing across sub-slices.
4. **Preemption & ISR Servicing**: $142 \ \mu\text{s}$ ($8.5\%$). Preemption by high-priority interrupts and `btController`.

### 4. Espressif Dual-Core JTAG Wrap-Around Bug (Issue #10604 / IDFGH-9216)
- **Root Cause**: On the ESP32-S3, the internal hardware TRAX trace memory buffer is $16 \ \text{KB}$. Dual-core OpenOCD streaming runs cleanly from byte 0 up to ~12 KB. However, when trace generation exceeds the $16 \ \text{KB}$ capacity and wraps around to byte 0 (`wr 7`), OpenOCD loses SEGGER packet boundary synchronization.
- **Symptoms**:
  - OpenOCD console reports: `Error: Incomplete block sz X, wr Y` and `Error: SEGGER: Unsupported predef event 0!`.
  - Exported CSV traces exhibit an unparseable wrap-around tail where timestamps jump discontinuously (e.g. jumping from ~40 ms to ~3.35 billion $\mu\text{s}$).
- **Two-Tier Mitigation**:
  1. **Capture Tier (12 KB Sweet Spot)**: Set `trace_size_bytes = 12000` in `esp sysview start file://core0.SVDat file://core1.SVDat 1 12000 -1`. This captures ~50 to 100 complete flight loop iterations and automatically terminates capture *before* buffer wrap-around occurs.
  2. **Analysis Tier (Sanitization Filter)**: Python analyzer scripts incorporate `clean_trace_tail()`, which discards any trailing records whose timestamps jump by $> 1 \ \text{s}$ or exceed $10^8 \ \mu\text{s}$ in millisecond-scale profiling sessions.

---

## 5. Standardized RTOS Timing Analysis Rules

All analyzer tools and developer reports MUST adhere to the following definitions:

1. **Average Runtime & WCET**:
   - Must measure the **full loop iteration** from `SYSVIEW_START` to `SYSVIEW_END` (total elapsed time including internal I2C/SPI yields and preemptions during that iteration).
   - Never measure runtime over isolated internal sub-slices.

2. **Loop Period & Running Frequency**:
   - Task Period ($T$) and Frequency ($f$) must be calculated between consecutive full loop iteration start timestamps (`Start Marker` or wake-up tick).

3. **Jitter Definition**:
   - Jitter is strictly defined as the **standard deviation ($\sigma$) of the full-loop period**:
     $$\text{Jitter} = \sigma_T = \sqrt{\frac{1}{N - 1} \sum_{i=1}^{N} (T_i - \bar{T})^2}$$

4. **Strict Uniform Units**:
   - All timing durations (Runtime, WCET, Period, Jitter, Blocked time) must be expressed in **microseconds ($\mu\text{s}$)**.
   - All frequencies must be expressed in **Hertz ($\text{Hz}$)**.
   - Mixed units (e.g. milliseconds alongside microseconds) are strictly prohibited.

5. **Trace Tail Wrap-Around Sanitization**:
   - Analyzer scripts must automatically detect and truncate corrupted wrap-around tails if $\Delta t > 10^6 \ \mu\text{s}$ ($> 1 \ \text{s}$) or $t > 10^8 \ \mu\text{s}$, retaining exclusively the valid contiguous steady-state flight window.

---

## 6. Steady-State Flight Loop Benchmarks (Dual-Core Trace Baseline)

Profiling clean steady-state traces (`core0.csv` and `core1.csv`) captured with the 12 KB sweet spot confirms robust real-time flight performance:

| Task Name | Core | Target Rate | Measured Freq | Measured Period ($T$) | Jitter ($\sigma$) | Avg Runtime | WCET | CPU Core Headroom |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **`ACCEL_GYRO_DAQ`** | Core 0 | $500 \ \text{Hz}$ | **$498.6 \ \text{Hz}$** | $2005.7 \ \mu\text{s}$ | **$51.2 \ \mu\text{s}$** | $720.6 \ \mu\text{s}$ ($36.1\%$) | $934.0 \ \mu\text{s}$ ($46.7\%$) | **$53.3\%$** ($1066.0 \ \mu\text{s}$) |
| **`MAG_DAQ`** | Core 1 | $100 \ \text{Hz}$ | **$100.0 \ \text{Hz}$** | $10000.0 \ \mu\text{s}$ | **$0.0 \ \mu\text{s}$** | $1583.0 \ \mu\text{s}$ ($15.8\%$) | $1583.0 \ \mu\text{s}$ ($15.8\%$) | **$84.2\%$** |
| **`BLE_VEC3_TRANS_`** | Core 0/1 | $100 \ \text{Hz}$ | **$100.0 \ \text{Hz}$** | $10000.0 \ \mu\text{s}$ | **$0.0 \ \mu\text{s}$** | $14.5 \ \mu\text{s}$ ($0.15\%$) | $15.0 \ \mu\text{s}$ ($0.15\%$) | **$> 99\%$** |

### Ground-Truth Cross-Verification
The SystemView JTAG measurements align closely with the onboard FreeRTOS hardware timer microsecond telemetry printed to the Serial Monitor:
- **Serial Monitor Ground Truth**: $T = 1999.06 \ \mu\text{s}$ ($f = 500.23 \ \text{Hz}$), Jitter $\sigma = 9.38 \ \mu\text{s}$.
- **SystemView JTAG Measurement**: $T = 2005.7 \ \mu\text{s}$ ($f = 498.6 \ \text{Hz}$), Jitter $\sigma = 51.2 \ \mu\text{s}$.
- **Headroom Margin**: Even under peak worst-case execution ($934.0 \ \mu\text{s}$), the 500 Hz flight loop retains $1066.0 \ \mu\text{s}$ ($53.3\%$) of CPU Core 0 headroom, guaranteeing ample execution margin for attitude estimators and cascaded PID rate controllers.

---

## 7. Python Profiling & Visualization Suite (`/analyzers`)

- `analyzers/sysview_core0_analyzer.py`:
  - Parses exported SystemView CSV traces for Core 0 (`core0.csv`).
  - Correlates markers to calculate full-loop periods, WCET, jitter ($\sigma$), and CPU Core 0 load.
  - Automatically exports structured markdown reports (`sysview_analysis_report.md`) and diagnostic plots (`sysview_analysis_plot.png`).
- `analyzers/sysview_dual_core_analyzer.py`:
  - Parses dual-core SystemView CSV traces simultaneously (`core0.csv` and `core1.csv`).
  - Implements `clean_trace_tail()` to automatically strip ring-buffer wrap-around corruption.
  - Correlates multi-core task executions, cross-core migrations, full-loop periods, frequency, jitter ($\sigma$), and dual-CPU core utilization (CPU 0, CPU 1, and aggregate dual-core load).
  - Automatically exports:
    - Structured Markdown report: `sysview_dual_core_report.md`
    - Dual-Core CPU Load & Timeline Visualization: `sysview_dual_core_plot.png`
    - 6-Panel Dual-Core Task Histograms: `task_timing_histograms.png` (Runtime distribution & Period distribution for `accel_gyro_daq_task`, `mag_daq_task`, and `ble_vec3_trans_task` across both cores).
  - Integrates steady-state verification against runtime Serial Monitor ground-truth timestamps.
- `analyzers/plot_slice_view.py`:
  - Generates high-resolution forensic timeline diagrams (`accel_gyro_interrupted_slice.png`) showing sub-slice segmentation, mutex contention, ISR preemption, and I2C hardware wait phases.
