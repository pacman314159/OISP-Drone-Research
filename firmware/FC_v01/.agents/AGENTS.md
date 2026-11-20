# Coding Conventions & Architecture Rules

## 0. Rule Precedence (Normative)

This document and the Zettel vault accumulate rules over time. The following precedence order is the only sanctioned way to resolve a conflict between two rules. It is deliberately conservative: precedence is an exception mechanism, not a routine escape hatch.

1. **Default — the code conforms to the rule.** A rule is not stale merely because it is old, or because the code violates it. A violation that passes none of the gates below is a **bug in the code**, and is fixed in code.
2. **Gate A — explicit supersession.** A newer decision overrides an older rule **only if** it names what it replaces (for example "supersedes `[Note 0002]` §61"), or the older note is already marked superseded in `documents/Firmware Structure.md` §8. Silent supersession is forbidden.
3. **Gate B — physical impossibility.** If an older rule cannot be satisfied by any implementation on the target hardware, that rule MUST be narrowed in the same commit, and the narrowing MUST state the impossibility explicitly. Example: "no target primitives in Layer 3" together with "ISRs must live in Layer 3" is unsatisfiable — an ISR cannot be written without target primitives.
4. **Gate C — registered exception.** A deviation that satisfies neither Gate A nor Gate B MUST be recorded in a named table inside `documents/Firmware Structure.md` (state which rule it deviates from and why the deviation is unavoidable). No such table exists yet — the first Gate C ruling creates it. Undocumented deviations are not permitted.
5. **Normative document.** `documents/Firmware Structure.md` is normative except where a non-superseded Zettel narrows it under Gates A–C.
6. **Recording.** When a gate is used, the amending note MUST state which gate applies and why. An audit that cannot name the gate has not resolved the conflict.

## 1. Naming Conventions
- **Variables & Functions**: `snake_case` (e.g., `uint32_t loop_counter`, `void calculate_pid()`)
- **Classes & Structs**: `PascalCase` (e.g., `class PIDController`, `struct IMUData`)
- **Constants & Enums**: `ALL_CAPS_SNAKE` (e.g., `MAX_MOTOR_COUNT`, `TASK_PRIORITY_HIGH`)
- **Header Files**: `.h` for every header, C and C++. `.hpp` is not used in this project. Header-only templates keep the `.h` extension (e.g., `matrix.h`, `ring_buffer.h`).

## 2. Embedded Real-Time Constraints & Architecture
- **Zero Dynamic Allocation**: No `malloc()`, `free()`, `new`, or `delete` inside flight loops.
- **Explicit Units**: All physical variables must explicitly state units via comments at declaration or documentation (e.g., `int16_t gyro; // LSB`, `float accel; // m/s^2`, `float temp; // degree C`).
- **Core IMU Loop Frequency**: 500 Hz (Strict zero dynamic memory allocation during flight loop).
- **FreeRTOS Task Scoping**: All FreeRTOS tasks must be declared exclusively inside `core/` (organized into 8 functional domains: `src/core/sensor/`, `src/core/control/`, `src/core/actuator/`, `src/core/math/`, `src/core/safety/`, `src/core/telemetry/`, `src/core/blackbox/`, `src/core/interface/`) or `app/` layers. Tasks must NEVER be declared in `drivers/` or lower layers. `src/core/actuator/` holds motor/ESC activation and mixing task runners; `src/core/safety/` holds pre-flight gate evaluation and safety-status aggregation.
- **Centralized Task Parameters**: Task names (`TASK_*_NAME`), task IDs (`enum TaskIDs`), stack sizes (`enum TaskSizesBytes`), priority levels (`enum TaskPriorities`), core affinity (`enum TaskCores`), and frequencies (`enum TaskFrequenciesHz`) MUST be declared centrally inside `include/config.h`. Each task runner in Layer 4 (`src/core/`) obtains its assigned `task_id` directly from `include/config.h` and receives driver instance pointers via `void* arg`.

## 3. Indentation
- **Indentation level**: 1 level corresponds with 2 spaces (1 tab)

## 4. Coding Style & Bracing Conventions
- **Single-Line Control Flow Bracing**: If an `if`, `else`, `while`, `for`, or `switch` statement contains only 1 statement/line inside its body (including nested single-statement control flows), omit the curly braces `{}`. (e.g., `if(condition) return false;`, `for(uint8_t j = 0; j < N; ++j) sum += a[j];`).
- **Function Declaration Bracing**: Functions and method bodies must have NO space between the closing parenthesis `)` and opening brace `{` (e.g., `bool empty(){` instead of `bool empty() {`).
- **Control Flow Keyword & Opening Brace Spacing**: Control flow statements (`if`, `while`, `for`, `switch`) must have NO space between the keyword and opening parenthesis `(`, and NO space between the closing parenthesis `)` and opening brace `{` (e.g., `if(condition){` instead of `if (condition) {`, `while(true){` instead of `while (true) {`).
- **`else` Keyword Spacing**: `else` statements attached to closing braces must have NO space around `else` (e.g., `}else{` instead of `} else {`, `}else if(condition){` instead of `} else if (condition) {`).

## 5. Tool Execution & Shell Runtime Rules
- **Prefer Native Agent Tools**: Always prefer native IDE tools (`read_file`, `search_files`, `find_files`, `edit_file`) over spawning terminal commands. Never spawn a terminal command just to read or locate a file.
- **Windows Quoting & Glob Disablement**: When using CLI tools (like `rg`) on Windows, NEVER use unescaped glob syntax in arguments (e.g. avoid `-g "sdkconfig*"` or wildcard parameters that trigger `os error 123`). Pass explicit file paths or rely on native IDE file search.
- **Enforce Self-Terminating Commands**: When terminal execution is necessary on Windows, always run commands via a non-hanging, terminating invocation (e.g., `cmd /c "<command>"` or wrap commands cleanly). Do not start persistent interactive subshells.
- **Output Truncation**: Constrain output buffers on commands likely to produce high line counts by piping or filtering (e.g., limit search hits or compiler warnings) to prevent agent context buffer saturation.
- **No PowerShell File cmdlets**: Never run slow PowerShell cmdlets (`Get-ChildItem`, `Get-Content`, `Select-String`) for search or inspection.

## 6. Build & PlatformIO Workflow
- **Build Invocation**: Use `pio run` from the project root (`firmware/FC_v01`). Specify the environment explicitly and always include `-j 4` (e.g., `pio run -e fc-v01 -j 4`) to prevent workstation thermal throttling.
- **Always Build Verbose**: Always append `-v` to `pio run` (`pio run -e fc-v01 -j 4 -v`). PlatformIO's non-verbose output swallows the underlying CMake/IDF error (e.g. "Cannot find component list file" gives no component path or preceding failure), forcing guesswork. Verbose is required for actionable diagnostics.
- **No Concurrent Tooling During Active Builds**: Do not run automated searches or sub-tasks while an active compilation is streaming to avoid process lockouts and disk I/O bottlenecks.
- **Targeted Compilation Checks**: For quick syntax checks, build only the active component or compilation unit before triggering a full binary link.

## 7. Agent Planning & Execution Discipline
- **Fail Fast on Shell Errors**: If a terminal command returns a non-zero exit code or `os error`, immediately stop execution and state the error. Do NOT attempt automatic sequential retry variations unless the root cause is resolved.
- **Direct Code Edits Over Exploration**: If file paths and targets are known, jump directly to editing rather than running exploratory shell commands (`dir`, `pwd`, `rg`).

## 8. SystemView Tracing & RTOS Timing Conventions
- **Full-Loop Iteration Boundary**: Timing measurements, `Average Runtime`, and `WCET` must always encompass the full task iteration (from `SYSVIEW_START` to `SYSVIEW_END`). Never measure task frequency or runtime over fragmented internal I2C/SPI sub-slices.
- **Period & Frequency Measurement**: Task execution period ($T$) and measured running frequency ($f$) must be calculated from consecutive full loop starts (`Start Marker` or wakeup timestamp).
- **Jitter Definition**: Jitter is strictly defined as the standard deviation of the averaged full-loop period.
- **Strict Units**: All timing statistics must be reported in **microseconds ($\mu\text{s}$)** and frequencies in **Hertz ($\text{Hz}$)** without mixing units.
- **Trace Tail Wrap-Around Sanitization**: Due to a known OpenOCD dual-core wrap-around race condition (Espressif Issue #10604 / IDFGH-9216), trace analyzer scripts must automatically strip the disconnected wrap-around tail if consecutive timestamps jump by $> 1\text{ s}$ or exceed $10^8\ \mu\text{s}$ in millisecond-scale sessions, preserving only the valid contiguous execution window.
- **SystemView JTAG Tracing Anti-Choking**: `CONFIG_SYSVIEW_BUF_WAIT_TMO=0` and `CONFIG_APPTRACE_SV_BUF_WAIT_TMO=0` must be enforced to prevent CPU stalls. High-frequency ISR and Timer events must be disabled in `sdkconfig.defaults`. Tasks using `vTaskDelayUntil` must include an anti-windup overrun guard (`if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;`).
- **Marker Coverage**: Every `TaskIDs` entry must have a `SYSVIEW_START`/`SYSVIEW_END` pair and a name row in `k_marker_names[]` (enforced by `static_assert` in `sysview_tracing.cpp`). Never put a blocking wait (`xQueueReceive` with timeout, `portMAX_DELAY`, `vTaskDelay`) inside a marker. `CONFIG_APPTRACE_SV_MAX_TASKS` must stay ≥ 32. See note 0007 §9.