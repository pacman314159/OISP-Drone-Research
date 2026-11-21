---
id: "0056"
title: Logging Is Advisory — SD Storage Health Never Gates Flight
author: Drone Research Team / OpenCode
date: 2026-11-21
time: 19:20:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#safety", "#preflight", "#logging", "#blackbox", "#storage"]
references:
  - "[[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]"
  - "[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]"
  - "[[0027-preflightSensorDataStreamCheckFsmArchitecture]]"
  - "[[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]"
  - "[[0036-layer4SafetyDomainAndPreflightGatesDecoupling]]"
  - "[[0052-ina226RemovalMatekPdbAndFiveGatePreflightPipeline]]"
---

# Zettel 0056: Logging Is Advisory — SD Storage Health Never Gates Flight

## 1. Metadata & Scope
- **ID**: `0056`
- **Author**: Drone Research Team / OpenCode
- **Date**: 2026-11-21
- **Time**: 19:20:00 +07:00
- **Status**: Approved by the owner (2026-11-21). Implementation pending compile verification.
- **Amends**: `[[0036-layer4SafetyDomainAndPreflightGatesDecoupling]]` — the storage-health placeholder that lived in the safety domain is deleted; the verdict now lives where the data is.
- **Scope**: pre-flight gate policy and the SD/Card blackbox path. No change to any of the five flight gates.

---

## 2. Context & Motivation

`preflight_checks.h` carried a declared-but-never-called `check_storage_health()` whose body returned `PASSED`. It was a placeholder that implied a sixth safety gate nobody had built, and it invited exactly the mistake this note prevents: a name shaped like a gate, sitting in the gate header. The five-gate pipeline (`[[0052-ina226RemovalMatekPdbAndFiveGatePreflightPipeline]]`) counts sensor stream, stationary, passive calibration, tilt and RC — storage was never among them.

Two ways out were available, and they are not equivalent:

1. **Promote it to a sixth gate.** A missing, full or unmounted card becomes a no-fly condition.
2. **Make it advisory.** Storage health is measured, reported and logged, but the aircraft may arm and fly without a blackbox.

The owner ruled for **(2)**, with the reasoning stated in one sentence: *without the logging system the drone can still fly.* A blackbox protects against the unknown; it is not itself a flight-critical sensor. A third option was then taken for the placeholder itself: **delete it**, so the safety header contains exactly the five real gates and nothing shaped like a sixth.

---

## 3. Architectural Decision

1. **Logging is advisory by design.** No SD card, a full card, a failed mount, or a write error MUST NOT prevent the `PREFLIGHT → READY` transition, and MUST NOT force `PREFLIGHT → DEVELOPMENT`. The five gates are unchanged.
2. **Storage health is measured and reported, not gated.** The SD logger task — the sole owner of the driver — publishes a snapshot: mounted vs absent, free bytes, and a tri-state verdict (`UNKNOWN` before evaluation, `HEALTHY`, `DEGRADED`). It is readable through `storage_health_snapshot()`. The gate-shaped `check_storage_health()` placeholder is **deleted**, so no consumer can mistake it for a mandatory check.
3. **The system says so out loud.** On boot, and on every transition into a degraded state, one `Serial` line names the condition *and* states that flight is still permitted — e.g. `[STORAGE] SD card absent — logging disabled, flight permitted`. A silent absent card is the failure mode this decision exists to prevent: an operator must never believe they are logging when they are not.
4. **The rationale lives here, not in the source.** Source files carry at most a one-line pointer to this note. Comment blocks explaining the policy in code are rejected: code states *what*, the ledger states *why* (see `.agents/AGENTS.md` §0 on precedence and `documents/notes/.agents/AGENTS.md` §3 on the architectural memory bank).
5. **This is not a licence for silent data loss.** Because logging is optional, the *disclosure* is mandatory: degraded storage must be visible in the boot log and in any status surface. Optional to fly, never optional to know.

---

## 4. What Still Gates Flight (unchanged)

| # | Gate | Blocking? |
|---|---|---|
| 1 | Sensor data stream (`check_sensor_data_stream`) | **Yes** |
| 2 | Stationary condition (`check_stationary_condition`) | **Yes** |
| 3 | Passive sensor calibration (`check_passive_sensor_calibration`) | **Yes** |
| 4 | Platform tilt (`check_platform_tilt`) | **Yes** |
| 5 | RC receiver signal (`check_rc_controller_signal`) | **Yes** |
| — | Storage health | **No — advisory via `storage_health_snapshot()`; no gate function exists** |

---

## 5. Consequences

- A logged flight with no card now looks different from a logged flight with a card only because of the log lines and the reported verdict — the state machine is identical. Debugging "why did the log stop" starts with the boot banner, not with a state transition.
- The blackbox path (`[[0018-sdCardLoggingRestorationAndUserButtonShutdown]]`) must tolerate a null/unmounted driver without blocking the logger task or the FSM.
- If a future requirement makes logging mandatory for certification or a competition rule, that is a **new decision** superseding this one — it must name this note (Gate A, `.agents/AGENTS.md` §0). It is not an acceptable "small tweak".

---

## 6. Cross-References & Traceability
- [[0036-layer4SafetyDomainAndPreflightGatesDecoupling]]: defines the safety domain and `SafetyStatus`; this note settles what the storage check inside it may influence.
- [[0052-ina226RemovalMatekPdbAndFiveGatePreflightPipeline]]: fixes the pipeline at five gates — this note keeps it there.
- [[0018-sdCardLoggingRestorationAndUserButtonShutdown]]: the logger this decision makes optional.
- [[0013-sdCardSpiStorageAndFlightLoggingArchitecture]]: SD HAL and rate structure the health check inspects.
- [[0027-preflightSensorDataStreamCheckFsmArchitecture]]: gate 1 shape, used as the model for reporting a verdict without gating.
- [[0031-sixGatePreflightTerminalDevelopmentAndTaskEncapsulationArchitecture]]: the original gate pipeline this decision explicitly does not extend.
