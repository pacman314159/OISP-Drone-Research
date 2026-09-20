# Zettelkasten Decision Notes Rules (`documents/notes/`)

This directory contains structural strategy notes and architectural decision records for the `FC_v01` firmware project using the Zettelkasten method.

---

## 1. Incremental Note Identification
- **Note ID Format**: 4-digit unique incremental integer ID starting from `0000` (e.g., `0000`, `0001`, `0002`, ...).
- **Filename Convention**: `<4-digit-id>-<shortDescriptiveName>.md` (e.g., `0000-firmwareStructureLayerDecision.md`).

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
1. **Centralized Middleware IPC**: All inter-task communication primitives (FreeRTOS mutexes, semaphores, queues, ring buffers) MUST be centralized inside Layer 3 (`src/middleware/`). Never scatter shared resource allocations or global IPC handles across drivers or core algorithms.
2. **Architectural Memory Bank**: Notes serve as the authoritative decision ledger. Future document generation (`System Life Cycle.md`, `RTOS Architecture.md`, `Firmware Structure.md`) and AI reasoning must trace back to these notes.
3. **Anti Over-Engineering**: Decisions should favor simple, proven, modular patterns over complex abstractions.
