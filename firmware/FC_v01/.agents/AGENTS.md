# Coding Conventions & Architecture Rules

## 1. Naming Conventions
- **Variables & Functions**: `snake_case` (e.g., `uint32_t loop_counter`, `void calculate_pid()`)
- **Classes & Structs**: `PascalCase` (e.g., `class PIDController`, `struct IMUData`)
- **Constants & Enums**: `ALL_CAPS_SNAKE` (e.g., `MAX_MOTOR_COUNT`, `TASK_PRIORITY_HIGH`)
- **Header Files**: `.hpp` for C++, `.h` for C/HAL interfaces.

## 2. Embedded Real-Time Constraints
- **Zero Dynamic Allocation**: No `malloc()`, `free()`, `new`, or `delete` inside flight loops.
- **Explicit Units**: All physical variables must explicitly state units via comments at declaration or documentation (e.g., `int16_t gyro; // LSB`, `float accel; // m/s^2`, `float temp; // degree C`).

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
- **Build Invocation**: Use `pio run` from the project root (`firmware/FC_v01`). Specify the environment explicitly (e.g., `pio run -e esp32-s3-devkitc-1`).
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