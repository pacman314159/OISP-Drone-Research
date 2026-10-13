# Graph Report - FCDatMon  (2026-10-02)

## Corpus Check
- cluster-only mode — file stats not available

## Summary
- 205 nodes · 377 edges · 12 communities (6 shown, 6 thin omitted)
- Extraction: 93% EXTRACTED · 7% INFERRED · 0% AMBIGUOUS · INFERRED: 25 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `4984f9f4`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- ui_callbacks.py
- _log_to_console
- control_panel.py
- main.py
- _rebuild_plot_ui
- ble_reader.py
- BLEReader
- SerialReader
- TelemetryDataManager
- **FCDatMon (Flight Controller Data Monitoring System)**
- plot_state_machine.py
- program_state_and_evts_fead5dad.md

## God Nodes (most connected - your core abstractions)
1. `_log_to_console()` - 20 edges
2. `update_ui_state()` - 15 edges
3. `_rebuild_plot_ui()` - 13 edges
4. `load_app_settings()` - 12 edges
5. `_manage_setup_callback()` - 12 edges
6. `create_control_panel()` - 12 edges
7. `BLEReader` - 11 edges
8. `SerialReader` - 10 edges
9. `_pair_ble_callback()` - 10 edges
10. `_apply_layout_callback()` - 10 edges

## Surprising Connections (you probably didn't know these)
- `_pair_ble_callback()` --calls--> `BLEReader`  [EXTRACTED]
  gui/ui_callbacks.py → core/ble_reader.py
- `_connect_callback()` --calls--> `SerialReader`  [EXTRACTED]
  gui/ui_callbacks.py → core/serial_reader.py
- `_forget_device_callback()` --calls--> `delete_paired_ble_device()`  [EXTRACTED]
  gui/ui_callbacks.py → core/state_manager.py
- `_ble_listbox_clicked()` --calls--> `load_paired_ble_device()`  [EXTRACTED]
  gui/ui_callbacks.py → core/state_manager.py
- `_build_section_additional_settings()` --calls--> `load_app_settings()`  [EXTRACTED]
  gui/control_panel.py → core/app_settings.py

## Import Cycles
- None detected.

## Communities (12 total, 6 thin omitted)

### Community 0 - "ui_callbacks.py"
Cohesion: 0.10
Nodes (14): load_app_settings(), save_app_settings(), delete_layout(), delete_paired_ble_device(), get_available_layouts(), _get_mac_filename(), load_layout(), load_paired_ble_device() (+6 more)

### Community 1 - "_log_to_console"
Cohesion: 0.12
Nodes (24): get_paired_ble_devices(), save_paired_ble_device(), _ble_listbox_clicked(), _clear_callback(), _connect_callback(), _on_freq_log(), _disconnect_callback(), _eval_pair_btn_state() (+16 more)

### Community 2 - "control_panel.py"
Cohesion: 0.12
Nodes (11): _build_console(), _build_section_additional_settings(), _build_section_connection(), _build_section_layout(), _build_section_payload_settings(), _build_section_session(), create_control_panel(), _setup_themes() (+3 more)

### Community 3 - "main.py"
Cohesion: 0.13
Nodes (9): create_plot_window(), _hex_to_rgba(), _setup_plot_theme(), update_plots(), _global_resize_callback(), _global_scroll_handler(), main(), resource_path() (+1 more)

### Community 4 - "_rebuild_plot_ui"
Cohesion: 0.15
Nodes (16): _add_series_callback(), _apply_layout_callback(), _delete_series_callback(), _fit_y_callback(), _fix_y_toggled(), _get_hex_color(), _hex_input_changed(), _hex_to_rgba() (+8 more)

### Community 9 - "**FCDatMon (Flight Controller Data Monitoring System)**"
Cohesion: 0.18
Nodes (10): **📦 Building an Executable**, **🚧 Current Status**, **Dependencies**, **Directory Structure**, **FCDatMon (Flight Controller Data Monitoring System)**, **⚙️ Installation & Setup**, **🚀 Key Features**, **Prerequisites** (+2 more)

## Knowledge Gaps
- **8 isolated node(s):** `Sheet: Sheet1`, `**📦 Building an Executable**`, `**🚧 Current Status**`, `**Dependencies**`, `**Directory Structure**` (+3 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 86 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **6 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `BLEReader` connect `BLEReader` to `ui_callbacks.py`, `_log_to_console`, `ble_reader.py`?**
  _High betweenness centrality (0.104) - this node is a cross-community bridge._
- **Why does `SerialReader` connect `SerialReader` to `ui_callbacks.py`, `_log_to_console`, `ble_reader.py`?**
  _High betweenness centrality (0.104) - this node is a cross-community bridge._
- **Why does `TelemetryDataManager` connect `TelemetryDataManager` to `ble_reader.py`?**
  _High betweenness centrality (0.087) - this node is a cross-community bridge._
- **Are the 9 inferred relationships involving `_rebuild_plot_ui()` (e.g. with `_add_series_callback()` and `_delete_series_callback()`) actually correct?**
  _`_rebuild_plot_ui()` has 9 INFERRED edges - model-reasoned connections that need verification._
- **What connects `Sheet: Sheet1`, `**📦 Building an Executable**`, `**🚧 Current Status**` to the rest of the system?**
  _8 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `ui_callbacks.py` be split into smaller, more focused modules?**
  _Cohesion score 0.10099573257467995 - nodes in this community are weakly interconnected._
- **Should `_log_to_console` be split into smaller, more focused modules?**
  _Cohesion score 0.11954022988505747 - nodes in this community are weakly interconnected._