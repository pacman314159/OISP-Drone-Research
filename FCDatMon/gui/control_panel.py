import json
import random
import datetime
import dearpygui.dearpygui as dpg
import serial.tools.list_ports
from core.serial_reader import SerialReader
from core.state_manager import save_layout, load_layout, get_available_layouts, delete_layout
from gui.plot_2d_manager import update_plots

# Constants
DEFAULT_SETUP_NAME = "FCDatMon Setup 01"
DEFAULT_PROTOCOL = "USB (Serial)"
DEFAULT_TARGET = "9DOF_IMU"
DEFAULT_LAYOUT = "1x2"

COLOR_TITLE = (0, 255, 0)
COLOR_SECTION_HEADING = (255, 255, 100) # Bright yellow for all sections
COLOR_SESSION_HEADING = (255, 255, 100) 
COLOR_H1 = (220, 200, 100)
COLOR_AXIS_LBL = (130, 200, 130)
COLOR_LOG = (255, 255, 100)

THEME_RED_BTN = "theme_red_btn"
THEME_GREEN_BTN = "theme_green_btn"

CURRENT_READER = None

WINDOW_WIDTH = 420
WINDOW_HEIGHT = 700

PLOT_CACHE = {}
PLOT_CHASE_ACTIVE = {}

PLOT_PALETTE = [
    # Top 20 (Most Used)
    "#E6194B", "#3CB44B", "#FFE119", "#4363D8", "#F58231",
    "#911EB4", "#46F0F0", "#F032E6", "#BCF60C", "#FABEBE",
    "#008080", "#E6BEFF", "#9A6324", "#FFFAC8", "#800000",
    "#AAFFC3", "#808000", "#FFD8B1", "#000075", "#808080",
    # 30 Additional Vibrant Colors
    "#FF5733", "#C70039", "#900C3F", "#581845", "#DAF7A6",
    "#2ECC71", "#27AE60", "#3498DB", "#2980B9", "#9B59B6",
    "#8E44AD", "#F1C40F", "#F39C12", "#E67E22", "#D35400",
    "#E74C3C", "#C0392B", "#1ABC9C", "#16A085", "#34495E",
    "#2C3E50", "#7F8C8D", "#BDC3C7", "#95A5A6", "#117864",
    "#7D3C98", "#D68910", "#BA4A00", "#7E5109", "#28B463"
]

def _setup_themes():
    with dpg.theme(tag="theme_folder_picker"):
        with dpg.theme_component(dpg.mvWindowAppItem):
            dpg.add_theme_color(dpg.mvThemeCol_WindowBg, (30, 30, 30, 255))
            dpg.add_theme_style(dpg.mvStyleVar_WindowPadding, 15, 15)
            
    with dpg.theme(tag=THEME_RED_BTN):
        with dpg.theme_component(dpg.mvButton):
            dpg.add_theme_color(dpg.mvThemeCol_Text, (255, 80, 80))

    with dpg.theme(tag=THEME_GREEN_BTN):
        with dpg.theme_component(dpg.mvButton):
            dpg.add_theme_color(dpg.mvThemeCol_Text, (80, 255, 80))

import re

def _hex_to_rgba(hex_str):
    """
    Safely parses a hexadecimal color string into an RGBA integer tuple used by DearPyGui.
    """
    hex_str = str(hex_str).strip().lstrip('#')
    if len(hex_str) != 6:
        return (255, 255, 255, 255)
    try:
        return (int(hex_str[0:2], 16), int(hex_str[2:4], 16), int(hex_str[4:6], 16), 255)
    except ValueError:
        return (255, 255, 255, 255)

def _log_to_console(msg):
    """
    Appends a formatted string to the application's internal system log window.
    """
    print(msg)
    current_text = dpg.get_value("console_output")
    dpg.set_value("console_output", f"{current_text}\n{msg}")

def _global_chase_callback(sender, app_data, user_data):
    """
    Toggles the chase state for ALL plots. If any plot is not chasing, it forces all to chase.
    Otherwise, it pauses chasing for all.
    """
    any_off = any(not PLOT_CHASE_ACTIVE.get(i, True) for i in PLOT_CACHE.keys())
    new_state = True if any_off else False
    for i in PLOT_CACHE.keys():
        PLOT_CHASE_ACTIVE[i] = new_state
        if dpg.does_item_exist(f"btn_local_chase_{i}"):
            dpg.bind_item_theme(f"btn_local_chase_{i}", THEME_GREEN_BTN if new_state else 0)
    if dpg.does_item_exist("btn_global_chase"):
        dpg.bind_item_theme("btn_global_chase", THEME_GREEN_BTN if new_state else 0)

def _global_fit_y_callback(sender, app_data, user_data):
    """
    Triggers DearPyGui's auto-fit function on all Y-axes across every rendered subplot,
    snapping the view to the current data boundaries.
    """
    for i in PLOT_CACHE.keys():
        if dpg.does_item_exist(f"plot_{i}_y"):
            dpg.fit_axis_data(f"plot_{i}_y")

def _local_chase_callback(sender, app_data, user_data):
    """
    Toggles the chase state for a specific individual plot. Updates the global chase button state 
    if this action causes all plots to be synced.
    """
    plot_id = user_data
    current_state = PLOT_CHASE_ACTIVE.get(plot_id, True)
    new_state = not current_state
    PLOT_CHASE_ACTIVE[plot_id] = new_state
    
    if dpg.does_item_exist(f"btn_local_chase_{plot_id}"):
        dpg.bind_item_theme(f"btn_local_chase_{plot_id}", THEME_GREEN_BTN if new_state else 0)
        
    all_on = all(PLOT_CHASE_ACTIVE.get(i, True) for i in PLOT_CACHE.keys())
    if dpg.does_item_exist("btn_global_chase"):
        dpg.bind_item_theme("btn_global_chase", THEME_GREEN_BTN if all_on else 0)
        
def _local_fit_y_callback(sender, app_data, user_data):
    """
    Triggers DearPyGui's auto-fit function for a specific plot's Y-axis.
    """
    plot_id = user_data
    if dpg.does_item_exist(f"plot_{plot_id}_y"):
        dpg.fit_axis_data(f"plot_{plot_id}_y")

def get_com_ports():
    """
    Scans the system for available serial COM ports.
    Returns a list of port names or a fallback list if scanning fails.
    """
    try:
        ports = [p.device for p in serial.tools.list_ports.comports()]
        if not ports:
            return ["No COM ports found"]
        return ports
    except Exception:
        return ["COM1", "COM2", "COM3"]

def _rescan_ports_callback(sender, app_data, user_data):
    """
    Callback triggered when the COM port dropdown is clicked. Re-scans for new devices.
    """
    if dpg.does_item_exist("combo_target_port"):
        ports = get_com_ports()
        dpg.configure_item("combo_target_port", items=ports)

def _protocol_changed_callback(sender, app_data):
    """
    Updates the connection UI fields based on whether USB (Serial) or a wireless protocol is selected.
    """
    protocol = dpg.get_value("combo_protocol")
    if protocol == "USB (Serial)":
        dpg.show_item("combo_target_port")
        dpg.show_item("combo_baudrate")
        dpg.hide_item("input_target")
        ports = get_com_ports()
        dpg.configure_item("combo_target_port", items=ports)
        if ports:
            dpg.set_value("combo_target_port", ports[0])
    else:
        dpg.show_item("input_target")
        dpg.hide_item("combo_target_port")
        dpg.hide_item("combo_baudrate")

def _connect_callback(sender, app_data, user_data):
    """
    Initiates a connection to the data source using the SerialReader engine.
    Updates the UI themes to reflect the connected state.
    """
    global CURRENT_READER
    protocol = dpg.get_value("combo_protocol")
    if protocol == "USB (Serial)":
        target = dpg.get_value("combo_target_port")
        baudrate = int(dpg.get_value("combo_baudrate"))
    else:
        target = dpg.get_value("input_target")
        baudrate = 115200
        
    _log_to_console(f"[*] Initiating {protocol} to '{target}' (Baud: {baudrate})...")
    
    if CURRENT_READER:
        CURRENT_READER.stop()
        
    CURRENT_READER = SerialReader(target, baudrate)
    success, msg = CURRENT_READER.start()
    if success:
        _log_to_console(f"[+] {msg}")
        dpg.bind_item_theme("btn_connect", 0)
        dpg.bind_item_theme("btn_terminate", THEME_RED_BTN)
    else:
        _log_to_console(f"[!] Failed to connect: {msg}")
        CURRENT_READER = None

def _disconnect_callback(sender, app_data, user_data):
    """
    Terminates the active connection and stops the SerialReader background thread safely.
    """
    global CURRENT_READER
    if CURRENT_READER:
        CURRENT_READER.stop()
        CURRENT_READER = None
        _log_to_console("[*] Connection terminated.")
        dpg.bind_item_theme("btn_connect", THEME_GREEN_BTN)
        dpg.bind_item_theme("btn_terminate", 0)
    else:
        _log_to_console("[*] No active connection to terminate.")

def _clear_callback(sender, app_data, user_data):
    """
    Flushes all historical data from the TelemetryDataManager and resets the plot buffers.
    """
    from core.data_manager import DATA_MANAGER
    DATA_MANAGER.clear()
    
    for i in list(PLOT_CACHE.keys()):
        cfg = PLOT_CACHE[i]
        series_list = cfg.get("series", [])
        for j in range(len(series_list)):
            series_tag = f"plot_{i}_series_{j+1}"
            if dpg.does_item_exist(series_tag):
                dpg.set_value(series_tag, [[], []])
    _log_to_console("[*] Plot data cleared.")

def _get_hex_color():
    """
    Returns a random hex color from the predefined PLOT_PALETTE to assign to new series.
    """
    return random.choice(PLOT_PALETTE)

def _save_ui_to_cache():
    """
    Reads all dynamic input fields (titles, series names, colors, etc.) from the DearPyGui
    item registry and stores them into the global PLOT_CACHE dictionary.
    """
    for i in PLOT_CACHE.keys():
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
                
            PLOT_CACHE[i] = {
                "title": dpg.get_value(f"p{i}_title"),
                "series": series_list
            }

def _apply_layout_callback(sender=None, app_data=None, user_data=None):
    """
    Calculates the number of required plots based on the selected Grid Layout (e.g., 2x3 = 6).
    Trims excess plots or initializes new ones, rebuilds the settings UI, and updates the grid.
    """
    _save_ui_to_cache()
    
    layout = dpg.get_value("combo_layout")
    rows, cols = map(int, layout.split('x'))
    count = rows * cols
    
    keys = sorted(list(PLOT_CACHE.keys()))
    for k in keys:
        if k > count:
            del PLOT_CACHE[k]
            
    for i in range(1, count + 1):
        if i not in PLOT_CACHE:
            PLOT_CACHE[i] = {
                "title": f"Plot {i}",
                "series": [
                    { "name": "Y-Axis", "unit": "ul", "width": 2.0, "color": _get_hex_color() }
                ]
            }
            PLOT_CHASE_ACTIVE[i] = True
            
    _rebuild_plot_ui()
    update_plots(layout, PLOT_CACHE)
    _log_to_console(f"[*] Grid set to {layout} ({count} plots)")

def _layout_changed_callback(sender, app_data, user_data):
    """
    Callback wrapper for when the Grid Layout dropdown value is changed by the user.
    """
    _apply_layout_callback()

def _plot_config_changed(sender=None, app_data=None, user_data=None):
    """
    Callback for any text/float modification in the plot settings menu.
    Immediately saves changes to cache and triggers a plot rebuild.
    """
    _save_ui_to_cache()
    layout = dpg.get_value("combo_layout")
    update_plots(layout, PLOT_CACHE)

def _hex_input_changed(sender, app_data, user_data):
    """
    Validates manual hex color text input and updates the corresponding UI color box.
    """
    i, j = user_data
    hex_val = str(app_data).strip()
    if not hex_val.startswith("#"):
        hex_val = "#" + hex_val
    if re.match(r'^#[0-9a-fA-F]{6}$', hex_val):
        if dpg.does_item_exist(f"p{i}_s{j}_color_box"):
            dpg.configure_item(f"p{i}_s{j}_color_box", default_value=_hex_to_rgba(hex_val))
        _plot_config_changed()

def _palette_color_clicked(sender, app_data, user_data):
    """
    Callback when a color is clicked inside the pop-up palette. Updates the hex input box.
    """
    i, j, color_hex = user_data
    if dpg.does_item_exist(f"p{i}_s{j}_color"):
        dpg.set_value(f"p{i}_s{j}_color", color_hex)
    if dpg.does_item_exist(f"p{i}_s{j}_color_box"):
        dpg.configure_item(f"p{i}_s{j}_color_box", default_value=_hex_to_rgba(color_hex))
    _plot_config_changed()

def _add_series_callback(sender, app_data, user_data):
    """
    Appends a new series configuration object to a specific plot and rebuilds the UI and grid.
    """
    i = user_data
    _save_ui_to_cache()
    if "series" not in PLOT_CACHE[i]:
        PLOT_CACHE[i]["series"] = []
    
    new_idx = len(PLOT_CACHE[i]["series"]) + 1
    PLOT_CACHE[i]["series"].append({
        "name": f"Series {new_idx}", 
        "unit": "ul", 
        "width": 2.0,
        "color": _get_hex_color()
    })
    _rebuild_plot_ui()
    _plot_config_changed()

def _delete_series_callback(sender, app_data, user_data):
    """
    Removes a specific series configuration from a plot and reconstructs the layout.
    """
    i, j = user_data
    _save_ui_to_cache()
    if "series" in PLOT_CACHE[i] and 0 <= j < len(PLOT_CACHE[i]["series"]):
        PLOT_CACHE[i]["series"].pop(j)
    _rebuild_plot_ui()
    layout_str = dpg.get_value("combo_layout")
    update_plots(layout_str, PLOT_CACHE)

def _rebuild_plot_ui():
    """
    Dynamically recreates the Plot Settings tree UI based on the current PLOT_CACHE state,
    including inputs for series titles, units, line widths, and colors.
    """
    expanded_states = {}
    for i in PLOT_CACHE.keys():
        tag = f"p{i}_tree"
        if dpg.does_item_exist(tag):
            expanded_states[i] = dpg.get_value(tag)
            
    dpg.delete_item("plot_settings_container", children_only=True)
    
    for i, cfg in PLOT_CACHE.items():
        is_open = expanded_states.get(i, True)
        with dpg.tree_node(label=f"Plot {i} Config", parent="plot_settings_container", tag=f"p{i}_tree", default_open=is_open):
            dpg.add_input_text(label="Title", default_value=cfg["title"], tag=f"p{i}_title", callback=_plot_config_changed)
            
            dpg.add_text("Series Configuration:", color=COLOR_AXIS_LBL)
            series_list = cfg.get("series", [])
            for j, s in enumerate(series_list):
                with dpg.group(horizontal=True):
                    dpg.add_text(f"#{j+1}")
                    dpg.add_input_text(width=45, default_value=s.get("name", ""), tag=f"p{i}_s{j+1}_name", callback=_plot_config_changed)
                    dpg.add_text("Unit:")
                    dpg.add_input_text(width=25, default_value=s.get("unit", ""), tag=f"p{i}_s{j+1}_unit", callback=_plot_config_changed)
                    
                with dpg.group(horizontal=True):
                    dpg.add_text("Width:")
                    dpg.add_input_text(width=35, default_value=str(s.get("width", 2.0)), tag=f"p{i}_s{j+1}_width", callback=_plot_config_changed)
                    dpg.add_text("Color:")
                    dpg.add_input_text(width=60, default_value=s.get("color", "#00FF00"), tag=f"p{i}_s{j+1}_color", callback=_hex_input_changed, user_data=(i, j+1))
                    dpg.add_color_button(default_value=_hex_to_rgba(s.get("color", "#00FF00")), tag=f"p{i}_s{j+1}_color_box", no_alpha=True, no_tooltip=True)
                    
                    with dpg.popup(f"p{i}_s{j+1}_color_box", mousebutton=dpg.mvMouseButton_Left):
                        dpg.add_text("Palette (50 Colors)")
                        for row in range(10):
                            with dpg.group(horizontal=True):
                                for col in range(5):
                                    c_hex = PLOT_PALETTE[row*5 + col]
                                    dpg.add_color_button(default_value=_hex_to_rgba(c_hex), callback=_palette_color_clicked, user_data=(i, j+1, c_hex), no_alpha=True)
                    
                    dpg.add_button(label="[X]", callback=_delete_series_callback, user_data=(i, j))
                    dpg.bind_item_theme(dpg.last_item(), THEME_RED_BTN)
            
            with dpg.group(horizontal=True):
                dpg.add_spacer(width=20)
                dpg.add_button(label="[+ Add Series]", callback=_add_series_callback, user_data=i)
            
            dpg.add_spacer(height=5)
            dpg.add_spacer(height=5)
            with dpg.group(horizontal=True):
                dpg.add_button(label="[Chase]", width=80, tag=f"btn_local_chase_{i}", callback=_local_chase_callback, user_data=i)
                if PLOT_CHASE_ACTIVE.get(i, True):
                    dpg.bind_item_theme(dpg.last_item(), THEME_GREEN_BTN)
                
                dpg.add_button(label="[Fit Y]", width=80, tag=f"btn_local_fit_y_{i}", callback=_local_fit_y_callback, user_data=i)
            dpg.add_spacer(height=10)

def _save_setup_callback(sender, app_data, user_data):
    """
    Validates user inputs and writes the entire current application state (protocol, layout, plot series) 
    to a JSON configuration file.
    """
    setup_name = dpg.get_value("input_setup_name").strip()
    if not setup_name:
        _log_to_console("[!] Error: Setup name cannot be empty.")
        return
        
    _save_ui_to_cache()
    
    # Input Validation
    for i, cfg in PLOT_CACHE.items():
        # Validate Line Color
        for j, s in enumerate(cfg.get("series", [])):
            hex_val = str(s.get("color", "")).strip()
            if not hex_val.startswith("#"):
                hex_val = "#" + hex_val
            if not re.match(r'^#[0-9a-fA-F]{6}$', hex_val):
                _log_to_console(f"[!] Error (Plot {i}, Series {j+1}): Invalid hex color '{s.get('color')}'. Format must be #RRGGBB.")
                return
            s["color"] = hex_val.upper()

    protocol = dpg.get_value("combo_protocol")
    target = dpg.get_value("combo_target_port") if protocol == "USB (Serial)" else dpg.get_value("input_target")
    baudrate = dpg.get_value("combo_baudrate") if protocol == "USB (Serial)" else "115200"
    setup_data = {
        "protocol": protocol,
        "target": target,
        "baudrate": baudrate,
        "layout": dpg.get_value("combo_layout"),
        "plots": PLOT_CACHE
    }
    try:
        save_layout(setup_name, setup_data)
        _log_to_console(f"[*] Saved config to '{setup_name}.json'")
        
        # Update the available setups listbox
        available_setups = get_available_layouts()
        if dpg.does_item_exist("list_available_setups"):
            dpg.configure_item("list_available_setups", items=available_setups)
            
    except Exception as e:
        _log_to_console(f"[!] Error saving config: {str(e)}")

def _load_setup_callback(sender, app_data, user_data):
    """
    Reads a saved JSON configuration file and restores the application state, connection parameters,
    and plot layouts.
    """
    setup_name = dpg.get_value("input_setup_name")
    try:
        setup_data = load_layout(setup_name)
        if setup_data is None:
            _log_to_console(f"[!] Configuration file '{setup_name}.json' not found.")
            return
            
        loaded_proto = setup_data.get("protocol", "BLE (ESP32-S3)")
        dpg.set_value("combo_protocol", loaded_proto)
        _protocol_changed_callback(None, None)
        if loaded_proto == "USB (Serial)":
            dpg.set_value("combo_target_port", setup_data.get("target", ""))
            dpg.set_value("combo_baudrate", setup_data.get("baudrate", "115200"))
        else:
            dpg.set_value("input_target", setup_data.get("target", ""))
            
        dpg.set_value("combo_layout", setup_data.get("layout", "2x3"))
        
        global PLOT_CACHE
        PLOT_CACHE.clear()
        
        loaded_plots = setup_data.get("plots", {})
        for k, v in loaded_plots.items():
            PLOT_CACHE[int(k)] = v
            PLOT_CHASE_ACTIVE[int(k)] = True
        
        _rebuild_plot_ui()
        update_plots(dpg.get_value("combo_layout"), PLOT_CACHE)
        _log_to_console(f"[*] Loaded config from '{setup_name}.json'")
    except Exception as e:
        _log_to_console(f"[!] Error loading config: {str(e)}")

def _build_section_connection():
    """
    Constructs the '1. HARDWARE CONNECTION' section of the control panel, containing
    protocol selection, port scanning, baudrate selection, and connect/disconnect buttons.
    """
    dpg.add_text(">> 1. HARDWARE CONNECTION", color=COLOR_SECTION_HEADING)
    dpg.add_combo(["BLE (ESP32-S3)", "LoRa (Serial)", "USB (Serial)"], default_value=DEFAULT_PROTOCOL, label="Protocol", tag="combo_protocol", callback=_protocol_changed_callback)
    
    ports = get_com_ports() if DEFAULT_PROTOCOL == "USB (Serial)" else []
    dpg.add_combo(ports, default_value=ports[0] if ports else "", label="Target / Port", tag="combo_target_port", show=(DEFAULT_PROTOCOL=="USB (Serial)"))
    
    with dpg.item_handler_registry(tag="combo_click_handler"):
        dpg.add_item_clicked_handler(callback=_rescan_ports_callback)
    dpg.bind_item_handler_registry("combo_target_port", "combo_click_handler")
    
    dpg.add_combo(["9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"], default_value="115200", label="Baud Rate", tag="combo_baudrate", show=(DEFAULT_PROTOCOL=="USB (Serial)"))
    dpg.add_input_text(default_value=DEFAULT_TARGET, label="Target / Port", tag="input_target", show=(DEFAULT_PROTOCOL!="USB (Serial)"))
    
    with dpg.group(horizontal=True):
        dpg.add_button(label="[CONNECT]", width=140, callback=_connect_callback, tag="btn_connect")
        dpg.bind_item_theme(dpg.last_item(), THEME_GREEN_BTN)
        
        dpg.add_button(label="[TERMINATE]", width=110, callback=_disconnect_callback, tag="btn_terminate")
        
        dpg.add_button(label="[CLEAR]", width=70, callback=_clear_callback)
    dpg.add_spacer(height=20)

def _build_section_layout():
    """
    Constructs the '2. DATA VISUALIZATION' section of the control panel, containing
    global view controls, grid layout dropdown, and individual plot configurations.
    """
    dpg.add_text(">> 2. DATA VISUALIZATION", color=COLOR_H1)
    
    with dpg.group(horizontal=True):
        dpg.add_button(label="[CHASE STREAM]", width=140, tag="btn_global_chase", callback=_global_chase_callback)
        # Check global state to apply theme on boot
        all_on = all(PLOT_CHASE_ACTIVE.get(i, True) for i in PLOT_CACHE.keys())
        if all_on:
            dpg.bind_item_theme("btn_global_chase", THEME_GREEN_BTN)
            
        dpg.add_button(label="[FIT VERTICAL]", width=140, tag="btn_global_fit_y", callback=_global_fit_y_callback)
    
    dpg.add_spacer(height=5)
    dpg.add_combo(["1x1", "1x2", "2x1", "2x2", "2x3", "3x2", "3x3"], default_value="1x2", label="Grid Layout", tag="combo_layout", callback=_layout_changed_callback)
    dpg.add_spacer(height=5)
    dpg.add_group(tag="plot_settings_container")
    dpg.add_spacer(height=20)

def _listbox_double_clicked(sender, app_data, user_data):
    """
    Callback triggered when a saved setup is double-clicked in the session listbox.
    Populates the setup name input field.
    """
    selected = dpg.get_value("list_available_setups")
    if selected:
        dpg.set_value("input_setup_name", selected)

def _delete_setup_callback(sender, app_data, user_data):
    """
    Deletes the currently selected setup configuration JSON file from disk.
    """
    selected = dpg.get_value("list_available_setups")
    if selected:
        deleted = delete_layout(selected)
        if deleted:
            _log_to_console(f"[*] Deleted config '{selected}.json'")
            available_setups = get_available_layouts()
            dpg.configure_item("list_available_setups", items=available_setups)
            if dpg.get_value("input_setup_name") == selected:
                dpg.set_value("input_setup_name", "")
        else:
            _log_to_console(f"[!] Could not find '{selected}.json' to delete.")

def _folder_picker_callback(sender, app_data):
    """
    Callback when the user selects a custom folder to save/load setup configurations.
    """
    from core.state_manager import set_config_dir, get_available_layouts
    folder_path = app_data['file_path_name']
    set_config_dir(folder_path)
    dpg.set_value("text_save_folder", f"Folder: {folder_path}")
    available_setups = get_available_layouts()
    dpg.configure_item("list_available_setups", items=available_setups)
    _log_to_console(f"[*] Save folder changed to: {folder_path}")

def _build_section_session():
    """
    Constructs the '3. SESSION STATE' section of the control panel, providing
    controls for saving, loading, and deleting layout configurations.
    """
    dpg.add_text(">> 3. SESSION STATE", color=COLOR_SECTION_HEADING)
    
    with dpg.file_dialog(directory_selector=True, show=False, callback=_folder_picker_callback, tag="folder_picker_dialog", width=500, height=400):
        pass

    dpg.add_input_text(default_value=DEFAULT_SETUP_NAME, label="Setup Name", tag="input_setup_name")
    
    with dpg.group(horizontal=True):
        dpg.add_button(label="[SAVE]", width=120, callback=_save_setup_callback)
        dpg.add_button(label="[LOAD]", width=120, callback=_load_setup_callback)
        
    dpg.add_text("Available Setups:", color=COLOR_AXIS_LBL)
    available_setups = get_available_layouts()

    with dpg.group(horizontal=True):
        dpg.add_text("Folder: default (/saved_setups)", tag="text_save_folder", wrap=250)
        dpg.add_button(label="[Select]", width=120, callback=lambda: dpg.show_item("folder_picker_dialog"))
    
    with dpg.group(horizontal=True):
        dpg.add_listbox(available_setups, width=240, num_items=8, tag="list_available_setups")
        dpg.add_button(label="[DELETE]", width=80, callback=_delete_setup_callback)
        dpg.bind_item_theme(dpg.last_item(), THEME_RED_BTN)
        
    with dpg.item_handler_registry(tag="listbox_double_click_handler"):
        dpg.add_item_double_clicked_handler(callback=_listbox_double_clicked)
    dpg.bind_item_handler_registry("list_available_setups", "listbox_double_click_handler")
    
    dpg.add_spacer(height=20)

def _data_folder_picker_callback(sender, app_data):
    """
    Callback when the user selects a custom folder to save raw telemetry CSV data.
    """
    folder_path = app_data['file_path_name']
    dpg.set_value("text_data_folder", f"Folder: {folder_path}")
    _log_to_console(f"[*] Data capture folder changed to: {folder_path}")

def _buffer_size_changed(sender, app_data, user_data):
    """
    Validates and updates the global telemetry buffer capacity. Resizes the deques dynamically.
    """
    val = int(app_data)
    if val < 100: val = 100
    if val > 50000: val = 50000
    dpg.set_value(sender, val)
    from core.data_manager import DATA_MANAGER
    DATA_MANAGER.resize_buffer(val)

def _build_section_additional_settings():
    """
    Constructs the '4. ADDITIONAL SETTINGS' section, containing buffer sizing and
    CSV data logging preferences.
    """
    dpg.add_text(">> 4. ADDITIONAL SETTINGS", color=COLOR_SECTION_HEADING)
    
    with dpg.file_dialog(directory_selector=True, show=False, callback=_data_folder_picker_callback, tag="data_folder_picker_dialog", width=500, height=400):
        pass

    with dpg.group(horizontal=True):
        dpg.add_text("Buffer Size (Samples):")
        dpg.add_input_int(default_value=10000, tag="input_buffer_size", callback=_buffer_size_changed, width=120)
        
    dpg.add_spacer(height=5)
    dpg.add_checkbox(label="Save Captured Data to Disk", default_value=False, tag="checkbox_save_data")

    with dpg.group(horizontal=True):
        dpg.add_text("Folder: default (/captured_data)", tag="text_data_folder", wrap=260)
        dpg.add_button(label="[Select]", width=100, callback=lambda: dpg.show_item("data_folder_picker_dialog"))
        
    current_time_str = datetime.datetime.now().strftime("FCDatMon %Y-%m-%d_%H-%M")
    dpg.add_input_text(default_value=current_time_str, label="File Name", tag="input_data_filename", width=250)
    
    dpg.add_spacer(height=20)


def _build_console():
    """
    Constructs the system log console text area at the bottom of the control panel.
    """
    dpg.add_text(">> SYSTEM LOG", color=COLOR_LOG)
    dpg.add_input_text(multiline=True, default_value="[OK] FCDatMon Initialized.", width=-1, height=100, readonly=True, tag="console_output")

def create_control_panel():
    """
    Master function that initializes the entire Control Panel window, its themes,
    and all functional sections.
    """
    _setup_themes()
    with dpg.window(label="Control Panel", tag="ControlPanel_Window", width=WINDOW_WIDTH, height=970, pos=[0,0], no_close=True, no_move=True, no_collapse=True, min_size=[WINDOW_WIDTH, 200], max_size=[WINDOW_WIDTH, 970]):
        with dpg.group(horizontal=True):
            dpg.add_text("FCDatMon v1.1", color=COLOR_TITLE)
            dpg.add_spacer(width=200)
            dpg.add_button(label="[<< Hide]", tag="btn_hide_panel")
        dpg.add_text("-" * 45, color=COLOR_TITLE)
        
        _build_section_connection()
        _build_section_layout()
        _build_section_session()
        _build_section_additional_settings()
        _build_console()
        
    _apply_layout_callback()

