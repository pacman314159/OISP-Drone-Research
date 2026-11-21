# Zettelkasten Decision Notes Rules (`documents/notes/`)

This directory contains structural strategy notes and architectural decision records for the `FC_v01` firmware project using the Zettelkasten method.

---

## 1. Incremental Note Identification
- **Note ID Format**: 4-digit unique incremental integer ID starting from `0000` (e.g., `0000`, `0001`, `0002`, ...).
- **Filename Convention**: `<4-digit-id>-<shortDescriptiveName>.md` (e.g., `0000-firmwareStructureLayerDecision.md`).
- **Retired IDs**: `0005` and `0014` do not exist and MUST NOT be reused. They were deleted before the vault grew a convention for tombstones; the gap is deliberate, not an omission.
- **Master index**: the traceability index is §8 of `documents/Firmware Structure.md`. No separate `INDEX.md` is maintained in this vault — do not create one.

---

## 2. Mandatory YAML Frontmatter Metadata Block
Every note created or modified in `documents/notes/` MUST begin with the following frontmatter metadata:

```yaml
---
id: "0000" # Unique 4-digit incremental ID
title: "Descriptive Note Title"
author: "Author Name"
date: YYYY-MM-DD
time: HH:MM:SS +TZ
tags: ["#tag1", "#tag2"]
references:
  - "[[note_id or document_title]]"
---
```

---

## 3. Core Principles & Purpose
1. **Centralized Middleware IPC**: All inter-task communication primitives (FreeRTOS mutexes, semaphores, queues, ring buffers) MUST be centralized inside Layer 3 (`src/middleware/`). Never scatter shared resource allocations or global IPC handles across drivers or core algorithms. *(Narrowed 2026-11-21 by `0002` Rule 6 as rewritten under `[[0053-layer3IpcAccessorSplitTaskRegistrySerializationAndHalOtaBoundary]]`: ownership stays centralized in `ipc.cpp`, while declarations are split across `include/ipc_types.h`, `middleware/ipc_handles.h` and `middleware/ipc_lifecycle.h`, and SPSC ring buffers may be accessed directly with their discipline stated at the declaration.)*
2. **Architectural Memory Bank**: Notes serve as the authoritative decision ledger. Future document generation (`System Life Cycle.md`, `RTOS Architecture.md`, `Firmware Structure.md`) and AI reasoning must trace back to these notes.
3. **Anti Over-Engineering**: Decisions should favor simple, proven, modular patterns over complex abstractions.
4. A note that supersedes another note or rule MUST name the note/section it supersedes. Silent supersession is not permitted — see `.agents/AGENTS.md` §0 Rule Precedence, Gate A.
