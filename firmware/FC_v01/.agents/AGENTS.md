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

## 4. FreeRTOS Task Scoping & Architecture Rules
- **Task Scoping**: All FreeRTOS tasks MUST be declared and defined strictly within `core/` (e.g., `src/core/daq/`, `src/core/control/`, `src/core/estimators/`, `src/core/telemetry/`) or `app/` layers. FreeRTOS task declarations are strictly prohibited inside `drivers/` or below.
- **`daq/` Definition**: `daq/` stands for **Data Acquisition** (`src/core/daq/`), housing high-frequency sensor sampling task runners (`daq_tasks.cpp`, `daq_tasks.h`).

## 5. Coding Style & Bracing Conventions
- **Single-Line Control Flow Bracing**: If an `if`, `else`, `while`, `for`, or `switch` statement contains only 1 statement/line inside its body (including nested single-statement control flows), omit the curly braces `{}`. (e.g., `if(condition) return false;`, `for(uint8_t j = 0; j < N; ++j) sum += a[j];`).
- **Function Declaration Bracing**: Functions and method bodies must have NO space between the closing parenthesis `)` and opening brace `{` (e.g., `bool empty(){` instead of `bool empty() {`).
- **Control Flow Keyword & Opening Brace Spacing**: Control flow statements (`if`, `while`, `for`, `switch`) must have NO space between the keyword and opening parenthesis `(`, and NO space between the closing parenthesis `)` and opening brace `{` (e.g., `if(condition){` instead of `if (condition) {`, `while(true){` instead of `while (true) {`).
- **`else` Keyword Spacing**: `else` statements attached to closing braces must have NO space around `else` (e.g., `}else{` instead of `} else {`, `}else if(condition){` instead of `} else if (condition) {`).
- **Namespace Indentation**: Everything declared/defined inside a `namespace` block (`namespace hal_simd { ... }`) must be indented by 1 level (2 spaces).


