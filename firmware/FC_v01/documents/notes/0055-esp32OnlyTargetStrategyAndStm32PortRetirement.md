---
id: "0055"
title: ESP32-S3-Only Target Strategy & Retirement of the STM32 Port Placeholder
author: Drone Research Team / OpenCode
date: 2026-11-21
time: 18:40:00 +07:00
tags: ["#zettelkasten", "#firmware", "#architecture", "#fc_v01", "#platform", "#hal", "#strategy", "#esp32", "#stm32"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0002-taskEnumsAndHalAbstractionDecision]]"
  - "[[0003-simdHalArchitectureAndLayeringRule]]"
  - "[[0048-halSpiInterfaceSegregationAndSdCardDriverDecoupling]]"
  - "[[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]]"
---

# Zettel 0055: ESP32-S3-Only Target Strategy & Retirement of the STM32 Port Placeholder

## 1. Metadata & Scope
- **ID**: `0055`
- **Author**: Drone Research Team / OpenCode
- **Date**: 2026-11-21
- **Time**: 18:40:00 +07:00
- **Status**: Approved (owner decision, 2026-11-21) & Implemented in the spec
- **Amends**: `[[0000-firmwareStructureLayerDecision]]` Layer 1 directory listing; `documents/Firmware Structure.md` §2 tree and §2.5 Layer 1 bullets (audit item **D3**).
- **Scope**: target strategy for the whole firmware; no code above Layer 1 is affected.

---

## 2. Context & Motivation

`0000` and the spec both reserved `src/platforms/targets/stm/` as a "placeholder for future STM32 porting" — a directory that has never existed. The claim was load-bearing in an unhealthy way: it implied a multi-target roadmap, so portability arguments were used to justify Layer 1 structures that had **no second target to validate against**. Portability that cannot be falsified is not a property; it is a design preference dressed as a constraint.

Meanwhile the real hardware is a single ESP32-S3 board with ESP-IDF, ESP-DSP, NimBLE and a RMT-based PPM decoder — all of it already target-bound.

---

## 3. Architectural Decision

1. **ESP32-S3 is the only supported target.** `src/platforms/targets/espressif/` is the only target directory. No STM32 tree exists, is scaffolded, or is reserved.
2. **The `stm/` placeholder is retired, not deferred.** Removed from the spec tree and from `0000`'s Layer 1 listing. A future port would begin by creating `src/platforms/targets/<soc>/` from scratch — reserving an empty directory in advance bought nothing and misled two audits.
3. **Layer 1 HAL abstraction remains mandatory — for a different reason.** `hal_i2c`, `hal_i2c_async`, `hal_pulse_tx`, `hal_spi`, `hal_ota`, `hal_simd`, `hal_matrix` stay. They are justified by **seam clarity and testability** (a Layer 2 driver can be exercised against a stub) and by keeping target headers out of public interfaces (`0002` Rules 3 and 6), *not* by a hypothetical STM32 port. Dropping STM32 does not weaken the HAL rule; it removes the excuse for over-generalizing it.
4. **A future migration, if it ever happens, is a bounded project.** Its contract: implement the Layer 1 interfaces in a new target directory; nothing in Layers 2–5 changes. If that ever requires changing a Layer 2–5 signature, the abstraction was wrong, and that is a finding, not a porting task.
5. **Spec text must not advertise capability that does not exist.** This is the same principle as the audit's "Deferred Subsystems" requirement (item **D8**): a roadmap claim belongs in a status table with an owner, not in a directory tree as a folder.

---

## 4. Impacted Files

| File | Change |
|---|---|
| `documents/Firmware Structure.md` §2 ASCII tree | `targets/stm/` line removed; `targets/espressif/` is the sole entry |
| `documents/Firmware Structure.md` §2.5 Layer 1 | STM32 placeholder bullet replaced by an explicit ESP32-only statement citing this note |
| `documents/Firmware Structure.md` §2.5 Layer 4 | `core/interface/` no longer described as "reserved" — it hosts the status-LED worker and button wiring (the other half of D3) |
| `documents/notes/0000-firmwareStructureLayerDecision.md` §3 | STM32 bullet replaced by a dated amendment citing this note |
| `include/config.h` | (same session, item C10, unrelated to this decision) two dead `*_FREQ_HZ` enum members deleted; 4 reserved pins tagged |

---

## 5. Verification

- `rg -i "stm32|targets/stm"` across `documents/` → only two historical hits remain, both in `0002` describing the *retired* header-ban text and one in this note.
- No `src/platforms/targets/stm/` directory exists on disk or in git history as a tree.
- No build file references a non-ESP target: `platformio.ini` defines `fc-v01` and `fc-v01-ota`, both ESP32-S3.

---

## 6. Cross-References & Traceability
- [[0000-firmwareStructureLayerDecision]]: Layer 1 listing amended; the 5-layer structure itself is unaffected.
- [[0002-taskEnumsAndHalAbstractionDecision]]: HAL-interaction and IPC rules still bind; this note narrows only *which* targets exist, not how layers talk.
- [[0003-simdHalArchitectureAndLayeringRule]]: SIMD abstraction is ESP-DSP-backed and stays inside Layer 1 — consistent with single-target reality.
- [[0048-halSpiInterfaceSegregationAndSdCardDriverDecoupling]]: the same HAL-vs-protocol-ownership test this decision relies on for justifying Layer 1 seams.
- [[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]]: the most recent HAL port added (`HAL_OTA`), and an example of the seam being justified by testability rather than portability.
