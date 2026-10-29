---
id: "0046"
title: FSM Direct Event Reporting, State Notification Pathways & Controlled Layer Inversion Exception
author: Drone Research Team / Developer
date: 2026-10-28
time: 01:15:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#fsm", "#layers", "#events", "#design-decision"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0020-fsmBackbonePreFlightSensorChecksAndActiveCalibrationArchitecture]]"
  - "[[0042-layer5ExclusiveTaskSuspensionAuthorityAndOtaDecoupling]]"
---

# Zettel 0046: FSM Direct Event Reporting, State Notification Pathways & Controlled Layer Inversion Exception

## 1. Metadata & Scope
- **ID**: `0046`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-28
- **Time**: 01:15:00 +07:00
- **Status**: Approved Architectural Exception & Formalized Design Rule
- **Module Context**: Layer 4 Domain Tasks (`src/core/telemetry/rc_receiver_task.cpp`, `src/core/sensor/calib_tasks.cpp`), Layer 5 Application Coordinator (`src/app/fsm.h`, `src/app/fsm.cpp`)

---

## 2. Context & Developer Design Intent
Under strict downward layered architecture (`documents/notes/0000-firmwareStructureLayerDecision.md`), Layer 4 (Core algorithms and worker tasks) must only depend downward on Layers 0–3 and never import Layer 5 (`src/app/`).

However, flight controller execution is intrinsically driven by the Finite State Machine (FSM). Modules in Layer 4 (such as the RC receiver detecting a failsafe loss of signal, or calibration tasks notifying step transitions) must immediately transmit discrete operational events to the central state machine to trigger safety transitions (e.g. `ARMED -> FAILSAFE`, `CALIB -> IDLE`).

The developer explicitly confirmed:
> *"FSM is special: if the modules in lower layers don't include it, how can they directly report errors and events so that the FSM may execute state changes? Layer inversion is allowed here."*

---

## 3. Core Dilemma: Strict Layering vs. Reactive Embedded State Machines
The question of how lower-level tasks signal high-level state coordinators represents one of the most classic architectural challenges in real-time embedded systems:

1. **Option A: Pure Layered Indirection (Observer Pattern / Function Pointers / Queue Bus)**:
   - *Mechanism*: Lower layers receive a callback pointer at boot or push events to a generic FreeRTOS event queue.
   - *Pros*: Preserves textbook layering; Layer 4 does not `#include "app/fsm.h"`.
   - *Cons*: Adds pointer indirection overhead, increases boot initialization boilerplate, consumes RAM for queue storage, and introduces risk of queue saturation or latency during urgent emergency events (e.g., immediate motor cut on failsafe).
2. **Option B: Controlled Layer Inversion (Direct FSM Event Dispatch)**:
   - *Mechanism*: Lower-layer worker tasks directly invoke `fsm_handle_event(EventCode)`.
   - *Pros*: Zero latency, zero queue memory allocation, deterministic execution, and crystal-clear call stacks during debugging.
   - *Cons*: Upward compile-time dependency (`#include "app/fsm.h"` in Layer 4).

---

## 4. Architectural Decision & Boundary Guardrails

To prevent direct FSM invocation from degrading into unstructured spaghetti coupling, the following guardrails are strictly enforced:

### Rule 1: FSM Header Isolation
- `src/app/fsm.h` must remain a **lean, interface-only header**. It must declare only:
  - FSM State Enums (`FlightState`)
  - FSM Event Enums (`FsmEvent`)
  - Event dispatch signatures (`bool fsm_handle_event(FsmEvent event)`)
- `fsm.h` must **never** include heavy application tasks, drivers, or peripheral coordinators.

### Rule 2: Unidirectional Event Ingestion Only
- Lower-layer tasks may only **push events** (`fsm_handle_event(...)`) or read read-only state (`fsm_get_state()`).
- Lower-layer tasks may **never mutate FSM internal tables** or execute state transitions directly; all transitions remain encapsulated within `src/app/fsm.cpp`.

### Rule 3: Distinct Authority Boundary
- As established in `[Note 0042]`, reporting an event (e.g. `EVENT_FAILSAFE_TRIGGERED`) is permissible for worker tasks. However, **executing global lifecycle actions** (such as suspending all background tasks) remains the exclusive authority of Layer 5 (`app.cpp`).
