---
id: "0036"
title: Layer 4 Safety Domain Scoping, Universal SafetyStatus & Pre-Flight Gates Decoupling Specification
author: Drone Research Team / Developer
date: 2026-10-25
time: 19:30:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#safety", "#fsm", "#preflight", "#refactoring"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0001-taskConfigMiddlewareIsrDecision]]"
  - "[[0016-coreDirectoryCompactionAndDomainConsolidation]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0027-preflightSensorDataStreamCheckFsmArchitecture]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
  - "[[0035-emergencyMotorDisarmMediatorAndLawOfDemeterDecision]]"
---

# Zettel 0036: Layer 4 Safety Domain Scoping, Universal SafetyStatus & Pre-Flight Gates Decoupling Specification

## 1. Metadata & Scope
- **ID**: `0036`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-25
- **Time**: 19:30:00 +07:00
- **Status**: Approved & Implemented
- **References**: `[Note 0000]`, `[Note 0001]`, `[Note 0016]`, `[Note 0020]`, `[Note 0027]`, `[Note 0031]`, `[Note 0035]`

---

## 2. Problem Statement & Motivation
Prior to this architectural refactoring, all 6 mandatory pre-flight safety gate functions (`sensor_data_stream_check`, `check_stationary_condition`, `check_passive_sensor_calibration`, `check_platform_tilt`, `check_power_initial_condition`, `check_rc_controller_signal`) were implemented directly inside Layer 5 (`src/app/fsm.cpp`):
1. **Application Layer Pollution**: `fsm.cpp` expanded to over 850 lines, intermingling high-level state transition rules and FreeRTOS task orchestration with multi-rate buffer latency math, vector trigonometry, and battery cell inference algorithms.
2. **Coupling of Mathematical Assertions to State Event Bitmasks**: The check routines directly mutated Layer 5 static event masks (`fsm_set_event`, `fsm_clear_event`), preventing unit testing of physical assertions in isolation.
3. **Absence of a Dedicated Safety Domain**: As reserved emergency flight states (`CRASHED`, `FREEFALL`, `LEVELOUT`) evolve into active detection routines, placing them in `src/app/` or `src/core/sensor/` would cause architectural boundary violations.

---

## 3. Core Architectural Decisions

### Rule 1: Establishment of `src/core/safety/` in Layer 4
A dedicated functional domain is established in Layer 4 (`src/core/safety/`), extending the 6-domain consolidation from `[Note 0016]`:
- `src/core/safety/safety_types.h`: Universal safety evaluation types.
- `src/core/safety/preflight_checks.h` & `preflight_checks.cpp`: Pure mathematical, temporal, and physical assertion routines for the 6 pre-flight gates.
- Reserved for future in-flight fail-safes (tip-over crash detection, zero-G freefall detection, attitude divergence).

### Rule 2: Universal `SafetyStatus` Evaluation Enum
All safety assertions return a tri-state enum:
```cpp
enum class SafetyStatus : uint8_t {
  PENDING = 0, // Still accumulating samples, warming up, or awaiting timeout
  PASSED,      // Physical & temporal conditions verified healthy
  FAILED       // Violation detected, timeout expired, or error latched
};
```
The routines maintain self-contained static temporal windows and latching states, returning `SafetyStatus` without directly accessing Layer 5 event bitmasks.

### Rule 3: Layer 5 FSM Preflight Coordination
`src/app/fsm.cpp` acts strictly as an event coordinator in `handle_state_preflight()`:
1. Samples `SafetyStatus` from each Layer 4 safety gate.
2. Maps statuses to FSM event masks (`EVT_*_OK/MET/PASS` vs `EVT_*_FAIL/LOW`) via `update_gate_event()`.
3. Passes stationary status to `check_passive_sensor_calibration(is_stationary)` to maintain causal evaluation.
4. Evaluates state transition rules (`evaluate_state_transitions()`).

This decouples Layer 4 from Layer 5, enforcing the strict downward dependency hierarchy.

---

## 4. Cross-References & Traceability
- Extends the 5-layer architecture of `[[0000-firmwareStructureLayerDecision]]` and core domain compaction in `[[0016-coreDirectoryCompactionAndDomainConsolidation]]`.
- Implements the 6-gate safety pipeline defined in `[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]` and stream validation from `[[0027-preflightSensorDataStreamCheckFsmArchitecture]]`.
- Preserves zero-jitter transition evaluation and mediator arming safety from `[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]` and `[[0035-emergencyMotorDisarmMediatorAndLawOfDemeterDecision]]`.
