# Drone Engineering & Software Architecture Rules

Welcome to the **00-drone-research-2026** team repository. These guidelines apply to all developers and AI pair programmers working on this codebase.

---

## 🎯 System Architecture Overview

1. **Flight Controller & Firmware**:
   - Primary RTOS: **FreeRTOS** / PX4 structure.
   - Core IMU Loop Frequency: **500 Hz** (Strict zero dynamic memory allocation during flight loop).
   - Control Architecture: **Cascaded PID** (Rate loop inside Angle/Attitude loop).
   - **FreeRTOS Task Scoping**: All FreeRTOS tasks must be declared exclusively inside `core/` (organized into 6 unified functional domains: `src/core/sensor/`, `src/core/control/`, `src/core/math/`, `src/core/telemetry/`, `src/core/blackbox/`, `src/core/interface/`) or `app/` layers. Tasks must NEVER be declared in `drivers/` or lower layers.
   - **`sensor/` Definition**: `sensor/` (`src/core/sensor/`) unifies high-frequency sensor Data Acquisition (DAQ) task runners (`daq_tasks.cpp`, `daq_tasks.h`) and sensor calibration routines.
   - **Centralized Task Parameters**: Task names (`TASK_*_NAME`), task IDs (`enum TaskIDs`), stack sizes (`enum TaskSizesBytes`), priority levels (`enum TaskPriorities`), core affinity (`enum TaskCores`), and frequencies (`enum TaskFrequenciesHz`) MUST be declared centrally inside `include/config.h`. Each task runner in Layer 4 (`src/core/`) obtains its assigned `task_id` directly from `include/config.h` (e.g. `TASK_ACCEL_GYRO_DAQ_ID`) and receives driver instance pointers via `void* arg`.

2. **Sensor Processing & RTOS Timing Analysis (`/analyzers`)**:
   - Barometer & Magnetometer data processing scripts must use FFT noise filtering routines (`baro_fft.py`, `imu_fft.py`, `mag_fft.py`).
   - **SystemView & RTOS Timing Analysis Conventions**:
     - **Full-Loop Runtime & WCET**: Average Runtime and Worst-Case Execution Time (WCET) MUST be measured strictly across full loop iterations (from `SYSVIEW_START` to `SYSVIEW_END` / complete iteration), never over fragmented internal sub-slices.
     - **Loop Period & Running Frequency**: Task Period ($T$) and Frequency ($f$) MUST be calculated between consecutive full loop iterations, never between internal queue yields.
     - **Jitter Definition**: Jitter is strictly defined as the standard deviation ($\sigma$) of the full-loop period.
     - **Strict Uniform Units**: All timing durations (Runtime, WCET, Period, Jitter, Blocked time) MUST be expressed in **microseconds ($\mu\text{s}$)**. All frequencies MUST be expressed in **Hertz ($\text{Hz}$)**. Do not mix units.
   - **SystemView JTAG Tracing Anti-Choking Rules**:
     - `CONFIG_SYSVIEW_BUF_WAIT_TMO=0` and `CONFIG_APPTRACE_SV_BUF_WAIT_TMO=0` must be enforced to prevent CPU stalls during buffer saturation.
     - High-frequency ISR and Timer events must be disabled in `sdkconfig.defaults` to prevent USB-JTAG bandwidth saturation (`Incomplete block sz X, wr Y`).
     - OpenOCD command syntax must strictly use active polling `esp sysview start <out0> <out1> 1 <size> -1` (never pass `-1` as the 3rd argument `poll_period_ms`).
     - High-frequency FreeRTOS tasks using `vTaskDelayUntil` must incorporate an anti-windup overrun guard (`if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;`).

3. **Documentation & Hardware References**:
   - Refer to system architecture specs in `system_architecture/System Architecture v01.pdf` and research papers in `research_docs/`.
   - Before implementing new sensor drivers or control algorithms, consult hardware datasheets in `datasheets/` and architectural decisions in `firmware/FC_v01/documents/notes/`.

---

## 🔒 Security Guidelines

- Never commit OAuth secrets, tokens, or `credentials.json` files to Git. All secrets must remain in `.agents/credentials.json` (ignored by `.gitignore`).

---

## ⚙️ Build & Command Execution Rules

- **Do NOT execute `pio run` or PlatformIO build/flash commands automatically.** The developer will manually trigger and run all builds. AI pair programmers must only create, modify, or inspect code files and documentation upon request.
- **Prohibition on PowerShell for File Inspection**: Do NOT execute PowerShell commands/cmdlets (e.g. `Get-ChildItem`, `Get-Content`, `Select-String`, PowerShell loops) for searching, listing, or reading files. Use native file-handling tool integrations (e.g. `view_file`, `replace_file_content`, `write_to_file`) or fast native CLI utilities (`rg`, `cat`) instead to prevent execution latency.

---

## 📝 Coding Style Rules

- **Single-Line Control Flow Bracing**: If an `if`, `else`, `while`, `for`, or `switch` statement contains only 1 statement/line inside its body (including nested single-statement control flows), omit the curly braces `{}`. (e.g., `if(condition) return false;`, `for(uint8_t j = 0; j < N; ++j) sum += a[j];`).
- **Function Declaration Bracing**: Functions and method bodies must have NO space between the closing parenthesis `)` and opening brace `{` (e.g., `void calculate_pid(){` instead of `void calculate_pid() {`).
- **Control Flow Keyword & Opening Brace Spacing**: Control flow statements (`if`, `while`, `for`, `switch`) must have NO space between the keyword and opening parenthesis `(`, and NO space between the closing parenthesis `)` and opening brace `{` (e.g., `if(condition){` instead of `if (condition) {`, `while(true){` instead of `while (true) {`).
- **`else` Keyword Spacing**: `else` statements attached to closing braces must have NO space around `else` (e.g., `}else{` instead of `} else {`, `}else if(condition){` instead of `} else if (condition) {`).
- **Namespace Indentation**: Everything declared/defined inside a `namespace` block (`namespace hal_simd { ... }`) must be indented by 1 level (2 spaces).
