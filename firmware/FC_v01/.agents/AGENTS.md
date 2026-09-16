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

## 5. Coding Style & Bracing Conventions
- **Single-Line `if` Statements**: If an `if` statement contains only 1 line of code inside its body, omit the curly braces `{}`. (e.g., `if(condition) return false;`).
- **Function Declaration Bracing**: Functions and method bodies must have NO space between the closing parenthesis `)` and opening brace `{` (e.g., `bool empty(){` instead of `bool empty() {`).
- **Control Flow Keyword & Opening Brace Spacing**: Control flow statements (`if`, `while`, `for`, `switch`) must have NO space between the keyword and opening parenthesis `(`, and NO space between the closing parenthesis `)` and opening brace `{` (e.g., `if(condition){` instead of `if (condition) {`, `while(true){` instead of `while (true) {`).


