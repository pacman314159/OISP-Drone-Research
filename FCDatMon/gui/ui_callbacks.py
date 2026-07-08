import re
import os
import random
import datetime
import dearpygui.dearpygui as dpg
import serial.tools.list_ports

from core.serial_reader import SerialReader
from core.ble_reader import BLEReader
from core.ble_scanner import run_ble_scan
from core.data_manager import DATA_MANAGER
from core.state_manager import (
    save_layout, load_layout, get_available_layouts, delete_layout,
    load_paired_ble_device, save_paired_ble_device, delete_paired_ble_device, get_paired_ble_devices,
    set_config_dir, set_paired_dir
)
from core.app_settings import load_app_settings, save_app_settings
from gui.plot_2d_manager import update_plots
import gui.ui_state as state

# ==============================================================================
# HELPER FUNCTIONS
# ==============================================================================

def _hex_to_rgba(hex_str):
    """Safely parses a hexadecimal color string into an RGBA integer tuple used by DearPyGui."""
    hex_str = str(hex_str).strip().lstrip('#')
    if len(hex_str) != 6: return (255, 255, 255, 255)
    try: return (int(hex_str[0:2], 16), int(hex_str[2:4], 16), int(hex_str[4:6], 16), 255)
    except ValueError: return (255, 255, 255, 255)

def _get_hex_color():
    """Returns a random hex color from the predefined PLOT_PALETTE to assign to new series."""
    return random.choice(state.PLOT_PALETTE)

def _log_to_console(msg):
    """Appends a formatted string to the application's internal system log window."""
    print(msg)
    if dpg.does_item_exist("console_output_text"):
        current_text = dpg.get_value("console_output_text")
        new_text = f"{current_text}\n{msg}"
        lines = new_text.split('\n')
        if len(lines) > 200: new_text = '\n'.join(lines[-200:])
        dpg.set_value("console_output_text", new_text)
    if dpg.does_item_exist("console_window"):
        max_scroll = dpg.get_y_scroll_max("console_window") + 500
        if dpg.does_item_exist("checkbox_autoscroll_log") and dpg.get_value("checkbox_autoscroll_log"):
            dpg.set_y_scroll("console_window", max_scroll)
        elif not dpg.does_item_exist("checkbox_autoscroll_log"):
            dpg.set_y_scroll("console_window", max_scroll)

def _clear_console_callback(sender, app_data, user_data):
    if dpg.does_item_exist("console_output_text"):
        dpg.set_value("console_output_text", "Console cleared.")

def get_com_ports():
    """Scans the system for available serial COM ports."""
    try:
        ports = []
        for p in serial.tools.list_ports.comports():
            desc = p.description if p.description and p.description != 'n/a' else ""
            if desc:
                desc = desc.replace(f" ({p.device})", "").strip()
                ports.append(f"{p.device} - {desc}")
            else:
                ports.append(p.device)
        return ports if ports else ["No COM ports found"]
    except Exception:
        return ["COM1", "COM2", "COM3"]

def update_ui_state(new_state: state.UIState):
    """Manages the overall UI state machine and button themes based on connection status."""
    state.CURRENT_UI_STATE = new_state
    
    for btn in ["btn_pair_ble", "btn_unpair_ble", "btn_connect", "btn_terminate", "btn_clear"]:
        if dpg.does_item_exist(btn): dpg.bind_item_theme(btn, 0)
        
    if new_state == state.UIState.INIT:
        if dpg.does_item_exist("input_ble_target"): dpg.set_value("input_ble_target", "")
        if dpg.does_item_exist("btn_pair_ble"): dpg.configure_item("btn_pair_ble", label="[PAIR]")
        if dpg.does_item_exist("btn_forget_ble"): dpg.bind_item_theme("btn_forget_ble", 0)
        if dpg.does_item_exist("list_paired_devices"): dpg.configure_item("list_paired_devices", items=get_paired_ble_devices())
        if dpg.does_item_exist("list_scanned_devices"): dpg.configure_item("list_scanned_devices", items=[])
            
    elif new_state == state.UIState.TARGET_SELECTED:
        if dpg.does_item_exist("btn_pair_ble"):
            dpg.configure_item("btn_pair_ble", label="[PAIR]")
            dpg.bind_item_theme("btn_pair_ble", state.THEME_GREEN_BTN)
            
    elif new_state == state.UIState.PAIRING:
        if dpg.does_item_exist("btn_pair_ble"):
            dpg.configure_item("btn_pair_ble", label="[PAIRING]")
            dpg.bind_item_theme("btn_pair_ble", state.THEME_YELLOW_BTN)
        if dpg.does_item_exist("btn_forget_ble"): dpg.bind_item_theme("btn_forget_ble", 0)
            
    elif new_state == state.UIState.PAIRED:
        if dpg.does_item_exist("btn_pair_ble"):
            dpg.configure_item("btn_pair_ble", label="[PAIRED]")
            dpg.bind_item_theme("btn_pair_ble", state.THEME_GREEN_BTN)
        if dpg.does_item_exist("btn_unpair_ble"): dpg.bind_item_theme("btn_unpair_ble", state.THEME_RED_BTN)
        if dpg.does_item_exist("btn_connect"): dpg.bind_item_theme("btn_connect", state.THEME_GREEN_BTN)
            
    elif new_state == state.UIState.STREAMING:
        if dpg.does_item_exist("btn_pair_ble"):
            dpg.configure_item("btn_pair_ble", label="[PAIRED]")
            dpg.bind_item_theme("btn_pair_ble", state.THEME_GREEN_BTN)
        if dpg.does_item_exist("btn_unpair_ble"): dpg.bind_item_theme("btn_unpair_ble", state.THEME_RED_BTN)
        if dpg.does_item_exist("btn_terminate"): dpg.bind_item_theme("btn_terminate", state.THEME_RED_BTN)
        if dpg.does_item_exist("btn_clear"): dpg.bind_item_theme("btn_clear", state.THEME_YELLOW_BTN)
            
    elif new_state == state.UIState.PAUSED:
        if dpg.does_item_exist("btn_pair_ble"):
            dpg.configure_item("btn_pair_ble", label="[PAIRED]")
            dpg.bind_item_theme("btn_pair_ble", state.THEME_GREEN_BTN)
        if dpg.does_item_exist("btn_unpair_ble"): dpg.bind_item_theme("btn_unpair_ble", state.THEME_RED_BTN)
        if dpg.does_item_exist("btn_connect"): dpg.bind_item_theme("btn_connect", state.THEME_GREEN_BTN)
        if dpg.does_item_exist("btn_clear"): dpg.bind_item_theme("btn_clear", state.THEME_YELLOW_BTN)

def _save_ui_to_cache():
    """Reads all dynamic input fields and stores them into the global PLOT_CACHE."""
    for i in state.PLOT_CACHE.keys():
        if dpg.does_item_exist(f"p{i}_title"):
            series_list = []
            j = 1
            while dpg.does_item_exist(f"p{i}_s{j}_unit"):
                series_list.append({
                    "name": dpg.get_value(f"p{i}_s{j}_name"),
                    "unit": dpg.get_value(f"p{i}_s{j}_unit"),
                    "width": dpg.get_value(f"p{i}_s{j}_width"),
                    "color": dpg.get_value(f"p{i}_s{j}_color")
                })
                j += 1
            state.PLOT_CACHE[i] = {
                "title": dpg.get_value(f"p{i}_title"),
                "fix_y": dpg.get_value(f"p{i}_fix_y") if dpg.does_item_exist(f"p{i}_fix_y") else False,
                "y_min": dpg.get_value(f"p{i}_y_min") if dpg.does_item_exist(f"p{i}_y_min") else "-100.0",
                "y_max": dpg.get_value(f"p{i}_y_max") if dpg.does_item_exist(f"p{i}_y_max") else "100.0",
                "series": series_list
            }

# ==============================================================================
# 1. HARDWARE CONNECTION CALLBACKS
# ==============================================================================

def _update_expected_format_callback(*args, **kwargs):
    if not dpg.does_item_exist("text_expected_format"): return
    
    protocol = dpg.get_value("combo_protocol")
    timestamp_mode = dpg.get_value("combo_timestamp")
    
    total_series = sum(len(config.get('series', [])) for config in state.PLOT_CACHE.values())
    
    if protocol == "BLE":
        format_str = "Binary (byte string): "
    elif protocol == "USB (Serial)":
        format_str = "ASCII (CSV): "
    else:
        format_str = ""
        
    parts = []
    if timestamp_mode == "TX":
        parts.append("uint32_t")
        
    for _ in range(total_series):
        parts.append("float")
        
    if not parts:
        dpg.set_value("text_expected_format", format_str + "No series configured")
    else:
        dpg.set_value("text_expected_format", format_str + ", ".join(parts))

def _protocol_changed_callback(sender, app_data, user_data):
    if state.CURRENT_READER:
        state.CURRENT_READER.stop()
        state.CURRENT_READER = None
        _log_to_console("[*] Protocol switched. Active connection closed.")
        
    protocol = dpg.get_value("combo_protocol")
    if protocol == "USB (Serial)":
        dpg.show_item("group_usb")
        dpg.hide_item("group_ble")
        dpg.hide_item("input_target")
        dpg.show_item("group_payload_settings")
        dpg.hide_item("input_batch_size")
        dpg.hide_item("text_payload_unsupported")
        ports = get_com_ports()
        dpg.configure_item("combo_target_port", items=ports)
        if ports: dpg.set_value("combo_target_port", ports[0])
        update_ui_state(state.UIState.INIT)
        _update_expected_format_callback()
    elif protocol == "BLE":
        dpg.hide_item("group_usb")
        dpg.show_item("group_ble")
        dpg.hide_item("input_target")
        dpg.show_item("group_payload_settings")
        dpg.show_item("input_batch_size")
        dpg.hide_item("text_payload_unsupported")
        update_ui_state(state.UIState.INIT)
        _update_expected_format_callback()
        devices = get_paired_ble_devices()
        if devices:
            dpg.set_value("list_paired_devices", devices[0])
            _ble_listbox_clicked("list_paired_devices", None, "double")

def _rescan_ports_callback(sender, app_data, user_data):
    if dpg.does_item_exist("combo_target_port"):
        dpg.configure_item("combo_target_port", items=get_com_ports())

def _scan_ble_callback(sender, app_data, user_data):
    label = dpg.get_item_label("btn_scan_ble")
    if label == "[SCAN]":
        dpg.configure_item("btn_scan_ble", label="[STOP SCAN]")
        dpg.configure_item("list_scanned_devices", items=["Scanning..."])
        timeout = load_app_settings().get("ble_scan_timeout", 5.0)
        _log_to_console(f"[*] Scanning for BLE devices... (Wait {timeout}s)")
        
        def _on_scan_complete(devices):
            def task():
                if dpg.does_item_exist("list_scanned_devices"):
                    dpg.configure_item("list_scanned_devices", items=devices if devices else ["No devices found."])
                    dpg.configure_item("btn_scan_ble", label="[SCAN]")
                    _log_to_console(f"[*] Scan complete. Found {len(devices)} devices.")
            state.UI_EVENT_QUEUE.append(task)
        run_ble_scan(_on_scan_complete, timeout=timeout)
    else:
        _log_to_console("[*] Please wait for current scan to finish.")

def _ble_listbox_clicked(sender, app_data, user_data):
    """Merged callback handling single/double clicks for both scanned and paired listboxes."""
    is_double_click = False
    if isinstance(user_data, tuple):
        is_double_click = (user_data[0] == "double")
        actual_sender = user_data[1]
    else:
        is_double_click = (user_data == "double")
        actual_sender = sender
        
    is_paired_list = (actual_sender == "list_paired_devices")
    
    selected = dpg.get_value(actual_sender)
    if not selected or selected in ("Scanning...", "No devices found."): return
        
    if is_double_click:
        if is_paired_list:
            dpg.set_value("list_scanned_devices", "") # deselect scanned
            data = load_paired_ble_device(selected)
            if data:
                name, mac = data.get("name", "Unknown"), data.get("mac", selected)
                target_str = f"{name} ({mac})" if name != "Unknown" and name != mac else mac
                dpg.set_value("input_ble_target", target_str)
                dpg.set_value("input_ble_service", data.get("service_uuid", state.DEFAULT_BLE_SERVICE_UUID))
                dpg.set_value("input_ble_char", data.get("char_uuid", state.DEFAULT_BLE_CHAR_UUID))
                update_ui_state(state.UIState.TARGET_SELECTED)
        else:
            dpg.set_value("list_paired_devices", "") # deselect paired
            dpg.set_value("input_ble_target", selected)
            update_ui_state(state.UIState.TARGET_SELECTED)
    else:
        # Single click logic
        if is_paired_list and state.CURRENT_UI_STATE not in (state.UIState.PAIRED, state.UIState.STREAMING, state.UIState.PAUSED):
            dpg.bind_item_theme("btn_forget_ble", state.THEME_YELLOW_BTN)
        else:
            dpg.bind_item_theme("btn_forget_ble", 0)

def _eval_pair_btn_state(sender, app_data, user_data):
    if state.CURRENT_UI_STATE in (state.UIState.PAIRED, state.UIState.STREAMING, state.UIState.PAUSED): return
    target = dpg.get_value("input_ble_target").strip()
    service = dpg.get_value("input_ble_service").strip()
    char = dpg.get_value("input_ble_char").strip()
    
    if target and service and char:
        update_ui_state(state.UIState.TARGET_SELECTED)
    else:
        if dpg.does_item_exist("btn_pair_ble"): dpg.bind_item_theme("btn_pair_ble", 0)

def _pair_ble_callback(sender, app_data, user_data):
    if state.CURRENT_UI_STATE == state.UIState.PAIRING:
        _log_to_console("[*] Aborting pairing process...")
        if state.CURRENT_READER:
            state.CURRENT_READER.stop()
            state.CURRENT_READER = None
        update_ui_state(state.UIState.TARGET_SELECTED)
        return
        
    if state.CURRENT_UI_STATE in (state.UIState.PAIRED, state.UIState.STREAMING, state.UIState.PAUSED): return
    target_raw = dpg.get_value("input_ble_target").strip()
    service_uuid = dpg.get_value("input_ble_service").strip()
    char_uuid = dpg.get_value("input_ble_char").strip()
    
    if not target_raw or not service_uuid or not char_uuid:
        _log_to_console("[ERROR] Target, Service UUID, and Char UUID must all be filled to pair!")
        return
        
    update_ui_state(state.UIState.PAIRING)
    _log_to_console(f"[*] Initiating pairing with {target_raw}...")
    
    match = re.search(r'\(([^)]+)\)', target_raw)
    mac = match.group(1) if match else target_raw
    name = target_raw.split(' (')[0].strip() if match else "Unknown"
    
    if state.CURRENT_READER: state.CURRENT_READER.stop()
        
    def _on_success():
        def task():
            save_paired_ble_device(target_raw, {"mac": mac, "name": name, "service_uuid": service_uuid, "char_uuid": char_uuid})
            _log_to_console(f"[*] Pairing success: {target_raw}")
            devices = get_paired_ble_devices()
            dpg.configure_item("list_paired_devices", items=devices)
            dpg.set_value("list_paired_devices", target_raw)
            update_ui_state(state.UIState.PAIRED)
        state.UI_EVENT_QUEUE.append(task)
        
    def _on_error(err_msg):
        def task():
            _log_to_console(f"[!] Pairing failed: {err_msg}")
            update_ui_state(state.UIState.TARGET_SELECTED)
        state.UI_EVENT_QUEUE.append(task)
        
    def _on_disconnect():
        def task():
            state.CURRENT_READER = None
            _log_to_console("[!] BLE Connection lost!")
            update_ui_state(state.UIState.INIT)
        state.UI_EVENT_QUEUE.append(task)
        
    def _on_freq_log(msg):
        state.UI_EVENT_QUEUE.append(lambda: _log_to_console(msg))
        
    timestamp_mode = dpg.get_value("combo_timestamp")
    timestamp_unit = dpg.get_value("combo_timestamp_unit")
    batch_size = dpg.get_value("input_batch_size")

    state.CURRENT_READER = BLEReader(mac, service_uuid, char_uuid, on_disconnect=_on_disconnect, on_connect_success=_on_success, on_connect_error=_on_error, timestamp_mode=timestamp_mode, timestamp_unit=timestamp_unit, batch_size=batch_size, log_frequency=state.LOG_FREQUENCY, freq_window_size=state.FREQUENCY_WINDOW_SIZE, on_freq_log=_on_freq_log)
    state.CURRENT_READER.start()

def _unpair_ble_callback(sender, app_data, user_data):
    if state.CURRENT_READER:
        state.CURRENT_READER.stop()
        state.CURRENT_READER = None
    _log_to_console("[*] Unpaired active BLE target.")
    update_ui_state(state.UIState.INIT)

def _forget_device_callback(sender, app_data, user_data):
    if state.CURRENT_UI_STATE in (state.UIState.PAIRED, state.UIState.STREAMING, state.UIState.PAUSED): return
    selected = dpg.get_value("list_paired_devices")
    if not selected: return
    delete_paired_ble_device(selected)
    _log_to_console(f"[*] Forgot BLE device: {selected}")
    devices = get_paired_ble_devices()
    dpg.configure_item("list_paired_devices", items=devices)
    dpg.set_value("list_paired_devices", "")
    dpg.bind_item_theme("btn_forget_ble", 0)

def _connect_callback(sender, app_data, user_data):
    protocol = dpg.get_value("combo_protocol")
    log_file_path = None
    if dpg.get_value("checkbox_save_data"):
        settings = load_app_settings()
        folder = settings.get("data_dir", os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "data", "captured_data"))
        filename = dpg.get_value("input_data_filename").strip()
        if not filename.endswith(".csv"): filename += ".csv"
        log_file_path = os.path.join(folder, filename)
    
    if protocol == "BLE":
        if state.CURRENT_UI_STATE == state.UIState.STREAMING: return
        if state.CURRENT_UI_STATE not in (state.UIState.PAIRED, state.UIState.PAUSED):
            _log_to_console("[*] You must Pair the device first.")
            return
        if state.CURRENT_READER and getattr(state.CURRENT_READER, 'is_running', False):
            if log_file_path and hasattr(state.CURRENT_READER, 'log_file_path'):
                state.CURRENT_READER.log_file_path = log_file_path
                if not state.CURRENT_READER.log_file:
                    os.makedirs(os.path.dirname(log_file_path), exist_ok=True)
                    state.CURRENT_READER.log_file = open(log_file_path, "a")
            state.CURRENT_READER.timestamp_mode = dpg.get_value("combo_timestamp")
            state.CURRENT_READER.timestamp_unit = dpg.get_value("combo_timestamp_unit")
            state.CURRENT_READER.batch_size = dpg.get_value("input_batch_size")
            state.CURRENT_READER.resume()
            for i in state.PLOT_CACHE.keys(): state.PLOT_CACHE[i]["trigger_y_snap"] = True
            update_ui_state(state.UIState.STREAMING)
            _log_to_console("[+] Started BLE data stream")
    else:
        if protocol == "USB (Serial)":
            target = dpg.get_value("combo_target_port").split(" - ")[0].strip()
            baudrate = int(dpg.get_value("combo_baudrate"))
        else:
            target = dpg.get_value("input_target")
            baudrate = 115200
            
        if state.CURRENT_READER and getattr(state.CURRENT_READER, 'port', None) == target and getattr(state.CURRENT_READER, 'baudrate', None) == baudrate and state.CURRENT_READER.is_running:
            if log_file_path and hasattr(state.CURRENT_READER, 'log_file_path'):
                state.CURRENT_READER.log_file_path = log_file_path
                if not state.CURRENT_READER.log_file:
                    os.makedirs(os.path.dirname(log_file_path), exist_ok=True)
                    state.CURRENT_READER.log_file = open(log_file_path, "a")
            state.CURRENT_READER.timestamp_mode = dpg.get_value("combo_timestamp")
            state.CURRENT_READER.timestamp_unit = dpg.get_value("combo_timestamp_unit")
            state.CURRENT_READER.resume()
            for i in state.PLOT_CACHE.keys(): state.PLOT_CACHE[i]["trigger_y_snap"] = True
            _log_to_console(f"[+] Resumed {protocol} streaming")
            update_ui_state(state.UIState.STREAMING)
        else:
            def _on_freq_log(msg):
                state.UI_EVENT_QUEUE.append(lambda: _log_to_console(msg))
                
            if state.CURRENT_READER: state.CURRENT_READER.stop()
            state.CURRENT_READER = SerialReader(target, baudrate, log_file_path=log_file_path, timestamp_mode=dpg.get_value("combo_timestamp"), timestamp_unit=dpg.get_value("combo_timestamp_unit"), log_frequency=state.LOG_FREQUENCY, freq_window_size=state.FREQUENCY_WINDOW_SIZE, on_freq_log=_on_freq_log)
            success, msg = state.CURRENT_READER.start()
            if success:
                _log_to_console(f"[+] Connected {protocol}")
                for i in state.PLOT_CACHE.keys(): state.PLOT_CACHE[i]["trigger_y_snap"] = True
                update_ui_state(state.UIState.STREAMING)
            else:
                _log_to_console(f"[!] Failed: {msg}")

def _disconnect_callback(sender, app_data, user_data):
    if state.CURRENT_READER:
        state.CURRENT_READER.pause()
        _log_to_console("[*] Connection paused.")
        update_ui_state(state.UIState.PAUSED)
        current_time_str = datetime.datetime.now().strftime("FCDatMon_%Y-%m-%d_%H-%M-%S")
        if dpg.does_item_exist("input_data_filename"): dpg.set_value("input_data_filename", current_time_str)
    else:
        _log_to_console("[*] No active connection to terminate.")

def _clear_callback(sender, app_data, user_data):
    DATA_MANAGER.clear()
    for i in list(state.PLOT_CACHE.keys()):
        for j in range(len(state.PLOT_CACHE[i].get("series", []))):
            series_tag = f"plot_{i}_series_{j+1}"
            if dpg.does_item_exist(series_tag): dpg.set_value(series_tag, [[], []])
    _log_to_console("[*] Plot data cleared.")

# ==============================================================================
# 2. DATA VISUALIZATION CALLBACKS
# ==============================================================================

def _apply_layout_callback(sender=None, app_data=None, user_data=None):
    _save_ui_to_cache()
    layout = dpg.get_value("combo_layout")
    rows, cols = map(int, layout.split('x'))
    count = rows * cols
    
    keys = sorted(list(state.PLOT_CACHE.keys()))
    for k in keys:
        if k > count: del state.PLOT_CACHE[k]
            
    for i in range(1, count + 1):
        if i not in state.PLOT_CACHE:
            state.PLOT_CACHE[i] = {
                "title": f"Plot {i}", "fix_y": False, "y_min": "-100.0", "y_max": "100.0",
                "series": [{ "name": "Y-Axis", "unit": "ul", "width": 2.0, "color": _get_hex_color() }]
            }
            state.PLOT_CHASE_ACTIVE[i] = True
            
    _rebuild_plot_ui()
    update_plots(layout, state.PLOT_CACHE)
    _update_expected_format_callback()
    _log_to_console(f"[*] Grid set to {layout} ({count} plots)")

def _layout_changed_callback(sender, app_data, user_data):
    _apply_layout_callback()

def _plot_config_changed(sender=None, app_data=None, user_data=None):
    _save_ui_to_cache()
    update_plots(dpg.get_value("combo_layout"), state.PLOT_CACHE)

def _fix_y_toggled(sender, app_data, user_data):
    i = user_data
    _save_ui_to_cache()
    if dpg.get_value(sender): state.PLOT_CACHE[i]["trigger_y_snap"] = True

def _hex_input_changed(sender, app_data, user_data):
    i, j = user_data
    hex_val = str(app_data).strip()
    if not hex_val.startswith("#"): hex_val = "#" + hex_val
    if re.match(r'^#[0-9a-fA-F]{6}$', hex_val):
        if dpg.does_item_exist(f"p{i}_s{j}_color_box"):
            dpg.configure_item(f"p{i}_s{j}_color_box", default_value=_hex_to_rgba(hex_val))
        _plot_config_changed()

def _palette_color_clicked(sender, app_data, user_data):
    i, j, color_hex = user_data
    if dpg.does_item_exist(f"p{i}_s{j}_color"): dpg.set_value(f"p{i}_s{j}_color", color_hex)
    if dpg.does_item_exist(f"p{i}_s{j}_color_box"): dpg.configure_item(f"p{i}_s{j}_color_box", default_value=_hex_to_rgba(color_hex))
    _plot_config_changed()

def _add_series_callback(sender, app_data, user_data):
    i = user_data
    _save_ui_to_cache()
    if "series" not in state.PLOT_CACHE[i]: state.PLOT_CACHE[i]["series"] = []
    state.PLOT_CACHE[i]["series"].append({"name": f"Series {len(state.PLOT_CACHE[i]['series']) + 1}", "unit": "ul", "width": 2.0, "color": _get_hex_color()})
    _rebuild_plot_ui()
    _plot_config_changed()
    _update_expected_format_callback()

def _delete_series_callback(sender, app_data, user_data):
    i, j = user_data
    _save_ui_to_cache()
    if "series" in state.PLOT_CACHE[i] and 0 <= j < len(state.PLOT_CACHE[i]["series"]):
        state.PLOT_CACHE[i]["series"].pop(j)
    _rebuild_plot_ui()
    update_plots(dpg.get_value("combo_layout"), state.PLOT_CACHE)
    _update_expected_format_callback()

def _freq_log_toggled(sender, app_data, user_data):
    state.LOG_FREQUENCY = dpg.get_value(sender)
    if state.CURRENT_READER:
        state.CURRENT_READER.log_frequency = state.LOG_FREQUENCY

def _freq_window_changed(sender, app_data, user_data):
    state.FREQUENCY_WINDOW_SIZE = dpg.get_value(sender)
    if state.CURRENT_READER:
        state.CURRENT_READER.freq_window_size = state.FREQUENCY_WINDOW_SIZE

def _toggle_chase_callback(sender, app_data, user_data):
    """Merged callback handling global and local chase toggles."""
    if user_data == "global":
        any_off = any(not state.PLOT_CHASE_ACTIVE.get(i, True) for i in state.PLOT_CACHE.keys())
        new_state = True if any_off else False
        for i in state.PLOT_CACHE.keys():
            state.PLOT_CHASE_ACTIVE[i] = new_state
            if new_state: state.PLOT_CACHE[i]["trigger_y_snap"] = True
            if dpg.does_item_exist(f"btn_local_chase_{i}"): dpg.bind_item_theme(f"btn_local_chase_{i}", state.THEME_GREEN_BTN if new_state else 0)
        if dpg.does_item_exist("btn_global_chase"): dpg.bind_item_theme("btn_global_chase", state.THEME_GREEN_BTN if new_state else 0)
    else:
        plot_id = user_data
        new_state = not state.PLOT_CHASE_ACTIVE.get(plot_id, True)
        state.PLOT_CHASE_ACTIVE[plot_id] = new_state
        if new_state: state.PLOT_CACHE[plot_id]["trigger_y_snap"] = True
        if dpg.does_item_exist(f"btn_local_chase_{plot_id}"): dpg.bind_item_theme(f"btn_local_chase_{plot_id}", state.THEME_GREEN_BTN if new_state else 0)
        all_on = all(state.PLOT_CHASE_ACTIVE.get(i, True) for i in state.PLOT_CACHE.keys())
        if dpg.does_item_exist("btn_global_chase"): dpg.bind_item_theme("btn_global_chase", state.THEME_GREEN_BTN if all_on else 0)

def _fit_y_callback(sender, app_data, user_data):
    """Merged callback handling global and local Y-axis auto-fit."""
    if user_data == "global":
        for i in state.PLOT_CACHE.keys():
            if not state.PLOT_CACHE[i].get("fix_y", False) and dpg.does_item_exist(f"plot_{i}_y"):
                dpg.fit_axis_data(f"plot_{i}_y")
    else:
        if not state.PLOT_CACHE[user_data].get("fix_y", False) and dpg.does_item_exist(f"plot_{user_data}_y"):
            dpg.fit_axis_data(f"plot_{user_data}_y")

def _time_window_callback(sender, app_data, user_data):
    """Merged callback handling global and local X-axis time window changes."""
    target, window_ms = user_data
    
    if target == "global":
        for i in state.PLOT_CACHE.keys():
            setattr(state, f"MANUAL_WINDOW_{i}", window_ms)
            if not state.PLOT_CHASE_ACTIVE.get(i, True): _toggle_chase_callback(None, None, i)
    else:
        setattr(state, f"MANUAL_WINDOW_{target}", window_ms)
        if not state.PLOT_CHASE_ACTIVE.get(target, True): _toggle_chase_callback(None, None, target)

def _rebuild_plot_ui():
    expanded_states = {i: dpg.get_value(f"p{i}_tree") for i in state.PLOT_CACHE.keys() if dpg.does_item_exist(f"p{i}_tree")}
    dpg.delete_item("plot_settings_container", children_only=True)
    
    for i, cfg in state.PLOT_CACHE.items():
        with dpg.tree_node(label=f"Plot {i} Config", parent="plot_settings_container", tag=f"p{i}_tree", default_open=expanded_states.get(i, True)):
            dpg.add_input_text(label="Title", default_value=cfg.get("title", f"Plot {i}"), tag=f"p{i}_title", callback=_plot_config_changed)
            with dpg.group(horizontal=True):
                dpg.add_checkbox(label="Fix Y", tag=f"p{i}_fix_y", default_value=cfg.get("fix_y", False), callback=_fix_y_toggled, user_data=i)
                dpg.add_text("Min:"); dpg.add_input_text(width=50, default_value=str(cfg.get("y_min", "-100.0")), tag=f"p{i}_y_min", callback=_plot_config_changed)
                dpg.add_text("Max:"); dpg.add_input_text(width=50, default_value=str(cfg.get("y_max", "100.0")), tag=f"p{i}_y_max", callback=_plot_config_changed)
            
            dpg.add_text("Series Configuration:", color=state.COLOR_AXIS_LBL)
            for j, s in enumerate(cfg.get("series", [])):
                with dpg.group(horizontal=True):
                    dpg.add_text(f"#{j+1}"); dpg.add_input_text(width=45, default_value=s.get("name", ""), tag=f"p{i}_s{j+1}_name", callback=_plot_config_changed)
                    dpg.add_text("Unit:"); dpg.add_input_text(width=25, default_value=s.get("unit", ""), tag=f"p{i}_s{j+1}_unit", callback=_plot_config_changed)
                with dpg.group(horizontal=True):
                    dpg.add_text("Width:"); dpg.add_input_text(width=35, default_value=str(s.get("width", 2.0)), tag=f"p{i}_s{j+1}_width", callback=_plot_config_changed)
                    dpg.add_text("Color:"); dpg.add_input_text(width=60, default_value=s.get("color", "#00FF00"), tag=f"p{i}_s{j+1}_color", callback=_hex_input_changed, user_data=(i, j+1))
                    dpg.add_color_button(default_value=_hex_to_rgba(s.get("color", "#00FF00")), tag=f"p{i}_s{j+1}_color_box", no_alpha=True, no_tooltip=True)
                    with dpg.popup(f"p{i}_s{j+1}_color_box", mousebutton=dpg.mvMouseButton_Left):
                        dpg.add_text("Color Palette")
                        for row in range(10):
                            with dpg.group(horizontal=True):
                                for col in range(5):
                                    c_hex = state.PLOT_PALETTE[row*5 + col]
                                    dpg.add_color_button(default_value=_hex_to_rgba(c_hex), callback=_palette_color_clicked, user_data=(i, j+1, c_hex), no_alpha=True)
                    dpg.add_button(label="[X]", callback=_delete_series_callback, user_data=(i, j))
                    dpg.bind_item_theme(dpg.last_item(), state.THEME_RED_BTN)
            
            with dpg.group(horizontal=True):
                dpg.add_spacer(width=20); dpg.add_button(label="[+ Add Series]", callback=_add_series_callback, user_data=i)
            dpg.add_spacer(height=10)
            with dpg.group(horizontal=True):
                dpg.add_button(label="[Chase]", width=75, tag=f"btn_local_chase_{i}", callback=_toggle_chase_callback, user_data=i)
                if state.PLOT_CHASE_ACTIVE.get(i, True): dpg.bind_item_theme(dpg.last_item(), state.THEME_GREEN_BTN)
                dpg.add_button(label="[Fit Y]", width=65, tag=f"btn_local_fit_y_{i}", callback=_fit_y_callback, user_data=i)
                dpg.add_button(label="1S", width=28, tag=f"btn_local_1s_{i}", callback=_time_window_callback, user_data=(i, 1000.0))
                dpg.add_button(label="2S", width=28, tag=f"btn_local_2s_{i}", callback=_time_window_callback, user_data=(i, 2000.0))
                dpg.add_button(label="5S", width=28, tag=f"btn_local_5s_{i}", callback=_time_window_callback, user_data=(i, 5000.0))
                dpg.add_button(label="10S", width=32, tag=f"btn_local_10s_{i}", callback=_time_window_callback, user_data=(i, 10000.0))
            dpg.add_spacer(height=10)

# ==============================================================================
# 3. SESSION STATE CALLBACKS
# ==============================================================================

def _handle_folder_selection(sender, app_data, user_data):
    """Unified handler for all folder pickers based on user_data."""
    folder_path = app_data['file_path_name']
    
    if user_data == "setup":
        set_config_dir(folder_path)
        dpg.set_value("text_save_folder", f"Folder: {folder_path}")
        dpg.configure_item("list_available_setups", items=get_available_layouts())
        _log_to_console(f"[*] Save folder changed to: {folder_path}")
    elif user_data == "data":
        dpg.set_value("text_data_folder", f"Folder: {folder_path}")
        _log_to_console(f"[*] Data capture folder changed to: {folder_path}")
        settings = load_app_settings()
        settings["data_dir"] = folder_path
        save_app_settings(settings)
    elif user_data == "paired":
        set_paired_dir(folder_path)
        dpg.set_value("text_paired_folder", f"Paired Devices: {folder_path}")
        if dpg.does_item_exist("list_paired_devices"):
            dpg.configure_item("list_paired_devices", items=get_paired_ble_devices())
            dpg.set_value("list_paired_devices", "")
        _log_to_console(f"[*] Paired devices folder changed to: {folder_path}")

def _listbox_double_clicked(sender, app_data, user_data):
    selected = dpg.get_value("list_available_setups")
    if selected: dpg.set_value("input_setup_name", selected)

def _manage_setup_callback(sender, app_data, user_data):
    """Merged callback handling Save, Load, and Delete for setups based on user_data."""
    setup_name = dpg.get_value("input_setup_name").strip()
    action = user_data
    
    if action == "save":
        if not setup_name:
            _log_to_console("[!] Error: Setup name cannot be empty.")
            return
        _save_ui_to_cache()
        for i, cfg in state.PLOT_CACHE.items():
            for j, s in enumerate(cfg.get("series", [])):
                hex_val = str(s.get("color", "")).strip()
                if not hex_val.startswith("#"): hex_val = "#" + hex_val
                if not re.match(r'^#[0-9a-fA-F]{6}$', hex_val):
                    _log_to_console(f"[!] Error (Plot {i}, Series {j+1}): Invalid hex color.")
                    return
                s["color"] = hex_val.upper()

        protocol = dpg.get_value("combo_protocol")
        target = dpg.get_value("combo_target_port") if protocol == "USB (Serial)" else dpg.get_value("input_target")
        baudrate = dpg.get_value("combo_baudrate") if protocol == "USB (Serial)" else "115200"
        setup_data = {
            "protocol": protocol, "target": target, "baudrate": baudrate, "layout": dpg.get_value("combo_layout"),
            "plots": state.PLOT_CACHE, "ble_target": dpg.get_value("input_ble_target"),
            "ble_service": dpg.get_value("input_ble_service"), "ble_char": dpg.get_value("input_ble_char"),
            "timestamp_mode": dpg.get_value("combo_timestamp"), "timestamp_unit": dpg.get_value("combo_timestamp_unit"),
            "batch_size": dpg.get_value("input_batch_size"),
            "log_frequency": state.LOG_FREQUENCY,
            "freq_window_size": state.FREQUENCY_WINDOW_SIZE
        }
        try:
            save_layout(setup_name, setup_data)
            _log_to_console(f"[*] Saved config to '{setup_name}.json'")
            if dpg.does_item_exist("list_available_setups"):
                dpg.configure_item("list_available_setups", items=get_available_layouts())
        except Exception as e: _log_to_console(f"[!] Error saving config: {str(e)}")
        
    elif action == "load":
        if not setup_name: return
        try:
            setup_data = load_layout(setup_name)
            if not setup_data:
                _log_to_console(f"[!] Configuration file '{setup_name}.json' not found.")
                return
            loaded_proto = setup_data.get("protocol", "BLE (ESP32-S3)")
            dpg.set_value("combo_protocol", loaded_proto)
            _protocol_changed_callback(None, None, None)
            if loaded_proto == "USB (Serial)":
                dpg.set_value("combo_target_port", setup_data.get("target", ""))
                dpg.set_value("combo_baudrate", setup_data.get("baudrate", "115200"))
            else:
                dpg.set_value("input_target", setup_data.get("target", ""))
                
            dpg.set_value("combo_layout", setup_data.get("layout", "2x3"))
            if "ble_target" in setup_data: dpg.set_value("input_ble_target", setup_data["ble_target"])
            if "ble_service" in setup_data: dpg.set_value("input_ble_service", setup_data["ble_service"])
            if "ble_char" in setup_data: dpg.set_value("input_ble_char", setup_data["ble_char"])
            if "timestamp_mode" in setup_data: dpg.set_value("combo_timestamp", setup_data["timestamp_mode"])
            if "timestamp_unit" in setup_data: dpg.set_value("combo_timestamp_unit", setup_data["timestamp_unit"])
            if "batch_size" in setup_data: dpg.set_value("input_batch_size", setup_data["batch_size"])
            
            if "log_frequency" in setup_data:
                state.LOG_FREQUENCY = setup_data["log_frequency"]
                dpg.set_value("checkbox_freq_log", state.LOG_FREQUENCY)
            if "freq_window_size" in setup_data:
                state.FREQUENCY_WINDOW_SIZE = setup_data["freq_window_size"]
                dpg.set_value("input_freq_window", state.FREQUENCY_WINDOW_SIZE)
            
            state.PLOT_CACHE.clear()
            for k, v in setup_data.get("plots", {}).items():
                state.PLOT_CACHE[int(k)] = v
                state.PLOT_CHASE_ACTIVE[int(k)] = True
            
            _rebuild_plot_ui()
            dpg.set_value("input_batch_size", setup_data.get("batch_size", 1))
            _apply_layout_callback()
            _update_expected_format_callback()
            _log_to_console(f"[*] Loaded config from '{setup_name}.json'")
        except Exception as e: _log_to_console(f"[!] Error loading config: {str(e)}")
        
    elif action == "delete":
        selected = dpg.get_value("list_available_setups")
        if selected and delete_layout(selected):
            _log_to_console(f"[*] Deleted config '{selected}.json'")
            dpg.configure_item("list_available_setups", items=get_available_layouts())
            if dpg.get_value("input_setup_name") == selected: dpg.set_value("input_setup_name", "")
        else: _log_to_console(f"[!] Could not find '{selected}.json' to delete.")

# ==============================================================================
# 4. ADDITIONAL SETTINGS CALLBACKS
# ==============================================================================

def _buffer_size_changed(sender, app_data, user_data):
    val = max(100, min(50000, int(app_data)))
    dpg.set_value(sender, val)
    DATA_MANAGER.resize_buffer(val)

def _ble_timeout_changed_callback(sender, app_data, user_data):
    val = max(1.0, min(60.0, float(app_data)))
    dpg.set_value(sender, val)
    settings = load_app_settings()
    settings["ble_scan_timeout"] = val
    save_app_settings(settings)
    _log_to_console(f"[*] BLE Scan Timeout updated to {val}s")
