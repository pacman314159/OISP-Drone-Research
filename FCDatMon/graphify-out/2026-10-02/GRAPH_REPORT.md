# Graph Report - FCDatMon  (2026-10-02)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 204 nodes · 377 edges · 14 communities (7 shown, 7 thin omitted)
- Extraction: 93% EXTRACTED · 7% INFERRED · 0% AMBIGUOUS · INFERRED: 25 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4984f9f4`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- ui_callbacks.py
- _log_to_console
- state_manager.py
- main.py
- BLEReader
- SerialReader
- TelemetryDataManager
- **FCDatMon (Flight Controller Data Monitoring System)**
- save_paired_ble_device
- _build_section_connection
- plot_state_machine.py
- ui_state.py
- program_state_and_evts_fead5dad.md

## God Nodes (most connected - your core abstractions)
1. `_log_to_console()` - 20 edges
2. `update_ui_state()` - 15 edges
3. `_rebuild_plot_ui()` - 13 edges
4. `_manage_setup_callback()` - 12 edges
5. `load_app_settings()` - 12 edges
6. `create_control_panel()` - 12 edges
7. `BLEReader` - 11 edges
8. `SerialReader` - 10 edges
9. `_apply_layout_callback()` - 10 edges
10. `_pair_ble_callback()` - 10 edges

## Surprising Connections (you probably didn't know these)
- `_pair_ble_callback()` --calls--> `BLEReader`  [EXTRACTED]
  gui/ui_callbacks.py → core/ble_reader.py
- `_connect_callback()` --calls--> `SerialReader`  [EXTRACTED]
  gui/ui_callbacks.py → core/serial_reader.py
- `_manage_setup_callback()` --calls--> `get_available_layouts()`  [EXTRACTED]
  gui/ui_callbacks.py → core/state_manager.py
- `_forget_device_callback()` --calls--> `get_paired_ble_devices()`  [EXTRACTED]
  gui/ui_callbacks.py → core/state_manager.py
- `_handle_folder_selection()` --calls--> `get_paired_ble_devices()`  [EXTRACTED]
  gui/ui_callbacks.py → core/state_manager.py

## Import Cycles
- None detected.

## Communities (14 total, 7 thin omitted)

### Community 0 - "ui_callbacks.py"
Cohesion: 0.09
Nodes (21): delete_layout(), load_layout(), save_layout(), update_plots(), _add_series_callback(), _apply_layout_callback(), _delete_series_callback(), _fit_y_callback() (+13 more)

### Community 1 - "_log_to_console"
Cohesion: 0.11
Nodes (24): run_ble_scan(), _scan_thread(), get_paired_ble_devices(), _ble_listbox_clicked(), _clear_callback(), _connect_callback(), _on_freq_log(), _disconnect_callback() (+16 more)

### Community 2 - "state_manager.py"
Cohesion: 0.13
Nodes (14): load_app_settings(), save_app_settings(), get_available_layouts(), set_config_dir(), set_paired_dir(), _build_console(), _build_section_additional_settings(), _build_section_layout() (+6 more)

### Community 3 - "main.py"
Cohesion: 0.15
Nodes (8): create_plot_window(), _hex_to_rgba(), _setup_plot_theme(), _global_resize_callback(), _global_scroll_handler(), main(), resource_path(), _toggle_panel()

### Community 8 - "**FCDatMon (Flight Controller Data Monitoring System)**"
Cohesion: 0.18
Nodes (10): **📦 Building an Executable**, **🚧 Current Status**, **Dependencies**, **Directory Structure**, **FCDatMon (Flight Controller Data Monitoring System)**, **⚙️ Installation & Setup**, **🚀 Key Features**, **Prerequisites** (+2 more)

### Community 9 - "save_paired_ble_device"
Cohesion: 0.22
Nodes (5): delete_paired_ble_device(), _get_mac_filename(), load_paired_ble_device(), save_paired_ble_device(), _forget_device_callback()

### Community 10 - "_build_section_connection"
Cohesion: 0.40
Nodes (3): _build_section_connection(), get_com_ports(), _rescan_ports_callback()

## Knowledge Gaps
- **8 isolated node(s):** `Sheet: Sheet1`, `**📦 Building an Executable**`, `**🚧 Current Status**`, `**Dependencies**`, `**Directory Structure**` (+3 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 85 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **7 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `BLEReader` connect `BLEReader` to `ui_callbacks.py`, `_log_to_console`, `ble_reader.py`?**
  _High betweenness centrality (0.105) - this node is a cross-community bridge._
- **Why does `SerialReader` connect `SerialReader` to `ui_callbacks.py`, `_log_to_console`, `ble_reader.py`?**
  _High betweenness centrality (0.104) - this node is a cross-community bridge._
- **Why does `TelemetryDataManager` connect `TelemetryDataManager` to `ble_reader.py`?**
  _High betweenness centrality (0.087) - this node is a cross-community bridge._
- **Are the 9 inferred relationships involving `_rebuild_plot_ui()` (e.g. with `_add_series_callback()` and `_delete_series_callback()`) actually correct?**
  _`_rebuild_plot_ui()` has 9 INFERRED edges - model-reasoned connections that need verification._
- **What connects `Sheet: Sheet1`, `**📦 Building an Executable**`, `**🚧 Current Status**` to the rest of the system?**
  _8 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `ui_callbacks.py` be split into smaller, more focused modules?**
  _Cohesion score 0.09175377468060394 - nodes in this community are weakly interconnected._
- **Should `_log_to_console` be split into smaller, more focused modules?**
  _Cohesion score 0.11494252873563218 - nodes in this community are weakly interconnected._