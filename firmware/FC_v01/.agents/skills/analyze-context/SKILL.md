---
name: analyze-context
description: >-
  Performs full architectural context analysis for FC_v01 by querying the graphify knowledge
  graph and scanning the Zettelkasten note vault. Run at the start of tasks or when user runs /analyze-context.
---

# FC_v01 Architectural & Codebase Context Analysis

Follow this workflow to establish complete system context before designing or modifying FC_v01 firmware.

## 1. Codebase Graph Traversal (Graphify)

- **Target Path**: `firmware/FC_v01`
- **Knowledge Graph Location**: `firmware/FC_v01/graphify-out/graph.json`
- **MCP Server Path Requirement**: When invoking Graphify MCP tools (`query_graph`, `shortest_path`, `get_node`, `get_neighbors`, `graph_stats`), ALWAYS pass:
  ```json
  "project_path": "d:/OneDrive_MSFT/Work/Drone_Research/firmware/FC_v01"
  ```
- **Actions**:
  1. Check `graph_stats` or query the target domain (e.g. sensor DAQ, telemetry, control loops).
  2. Map dependencies between Layer 1 (HAL), Layer 2 (Drivers), Layer 3 (Core/Middleware), and Layer 5 (App).
  3. Identify god nodes and FreeRTOS task boundaries.

## 2. Zettelkasten Note Vault & ADR Scan (Zettel)

- **Vault Location**: `firmware/FC_v01/documents/notes/` and `system_architecture/System Architecture v01.pdf`
- **Actions**:
  1. Search for existing Architectural Decision Records (ADRs) related to the topic (e.g., `0000`-`0004` 5-layer architecture, calibration state machines, IPC queues).
  2. Verify real-time constraints:
     - 500 Hz IMU rate loop timing budget.
     - Strict zero dynamic allocation (`malloc`/`new`) in flight loops.
     - Centralized task parameters in `include/config.h`.
     - Queue isolation and non-blocking IPC pushes.
  3. Trace bidirectional wikilinks (`[[...]]`) to prevent regressions against prior design choices.

## 3. Executive Synthesis

Summarize findings before proceeding with implementation or planning (`/plan`):
- **Core Components**: Affected files, tasks, and queues.
- **Governing ADRs**: Relevant design notes and constraints.
- **Hardware/RTOS Invariants**: Bus addresses, core affinity, priorities, and buffer depths.
