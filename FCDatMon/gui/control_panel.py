import json
import random
import datetime
import dearpygui.dearpygui as dpg
import serial.tools.list_ports
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
COLOR_AXIS_LBL = (150, 255, 150)
COLOR_LOG = (255, 255, 100)

THEME_RED_BTN = "theme_red_btn"
THEME_GREEN_BTN = "theme_green_btn"

WINDOW_WIDTH = 420
WINDOW_HEIGHT = 700

PLOT_CACHE = {}

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
    hex_str = str(hex_str).strip().lstrip('#')
    if len(hex_str) != 6:
        return (255, 255, 255, 255)
    try:
        return (int(hex_str[0:2], 16), int(hex_str[2:4], 16), int(hex_str[4:6], 16), 255)
    except ValueError:
        return (255, 255, 255, 255)

def _log_to_console(message):
    current_text = dpg.get_value("console_output")
    dpg.set_value("console_output", f"{current_text}\n{message}")

def get_com_ports():
    try:
        ports = [p.device for p in serial.tools.list_ports.comports()]
        if not ports:
            return ["No COM ports found"]
        return ports
    except Exception:
        return ["COM1", "COM2", "COM3"]

def _rescan_ports_callback(sender, app_data, user_data):
    if dpg.does_item_exist("combo_target_port"):
        ports = get_com_ports()
        dpg.configure_item("combo_target_port", items=ports)

def _protocol_changed_callback(sender, app_data):
    protocol = dpg.get_value("combo_protocol")
    if protocol == "USB (Serial)":
        dpg.show_item("combo_target_port")
        dpg.hide_item("input_target")
        ports = get_com_ports()
        dpg.configure_item("combo_target_port", items=ports)
        if ports:
            dpg.set_value("combo_target_port", ports[0])
    else:
        dpg.show_item("input_target")
        dpg.hide_item("combo_target_port")

def _connect_callback(sender, app_data, user_data):
    protocol = dpg.get_value("combo_protocol")
    if protocol == "USB (Serial)":
        target = dpg.get_value("combo_target_port")
    else:
        target = dpg.get_value("input_target")
    _log_to_console(f"[*] Initiating {protocol} to '{target}'...")

def _disconnect_callback(sender, app_data, user_data):
    _log_to_console("[*] Connection terminated.")

def _get_hex_color():
    return random.choice(PLOT_PALETTE)

def _save_ui_to_cache():
    for i in list(PLOT_CACHE.keys()):
        if dpg.does_item_exist(f"p{i}_title"):
            series_list = []
            j = 0
            while True:
                name_tag = f"p{i}_s{j}_name"
                if not dpg.does_item_exist(name_tag):
                    break
                s_name = dpg.get_value(name_tag)
                s_unit = dpg.get_value(f"p{i}_s{j}_unit")
                s_color = dpg.get_value(f"p{i}_s{j}_color")
                series_list.append({"name": s_name, "unit": s_unit, "color": s_color})
                j += 1
                
            PLOT_CACHE[i] = {
                "title": dpg.get_value(f"p{i}_title"),
                "v_auto_scale": dpg.get_value(f"p{i}_v_auto_scale"),
                "v_min": dpg.get_value(f"p{i}_v_min"),
                "v_max": dpg.get_value(f"p{i}_v_max"),
                "h_unit": dpg.get_value(f"p{i}_h_unit"),
                "series": series_list
            }

def _apply_layout_callback(sender=None, app_data=None, user_data=None):
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
                "v_auto_scale": False,
                "v_min": -180.0, "v_max": 180.0,
                "h_unit": "seconds",
                "series": [
                    { "name": "Y-Axis", "unit": "u", "color": _get_hex_color() }
                ]
            }
            
    _rebuild_plot_ui()
    update_plots(layout, PLOT_CACHE)
    _log_to_console(f"[*] Grid set to {layout} ({count} plots)")

def _plot_config_changed(sender=None, app_data=None, user_data=None):
    _save_ui_to_cache()
    layout = dpg.get_value("combo_layout")
    update_plots(layout, PLOT_CACHE)

def _hex_input_changed(sender, app_data, user_data):
    i, j = user_data
    hex_val = str(app_data).strip()
    if not hex_val.startswith("#"):
        hex_val = "#" + hex_val
    if re.match(r'^#[0-9a-fA-F]{6}$', hex_val):
        if dpg.does_item_exist(f"p{i}_s{j}_color_box"):
            dpg.configure_item(f"p{i}_s{j}_color_box", default_value=_hex_to_rgba(hex_val))
        _plot_config_changed()

def _palette_color_clicked(sender, app_data, user_data):
    i, j, color_hex = user_data
    if dpg.does_item_exist(f"p{i}_s{j}_color"):
        dpg.set_value(f"p{i}_s{j}_color", color_hex)
    if dpg.does_item_exist(f"p{i}_s{j}_color_box"):
        dpg.configure_item(f"p{i}_s{j}_color_box", default_value=_hex_to_rgba(color_hex))
    _plot_config_changed()

def _add_series_callback(sender, app_data, user_data):
    i = user_data
    _save_ui_to_cache()
    if "series" not in PLOT_CACHE[i]:
        PLOT_CACHE[i]["series"] = []
    
    new_idx = len(PLOT_CACHE[i]["series"]) + 1
    PLOT_CACHE[i]["series"].append({
        "name": f"Series {new_idx}", 
        "unit": "u", 
        "color": _get_hex_color()
    })
    _rebuild_plot_ui()
    _plot_config_changed()

def _delete_series_callback(sender, app_data, user_data):
    i, j = user_data
    _save_ui_to_cache()
    if "series" in PLOT_CACHE[i] and 0 <= j < len(PLOT_CACHE[i]["series"]):
        PLOT_CACHE[i]["series"].pop(j)
    _rebuild_plot_ui()
    _plot_config_changed()

def _auto_scale_changed_callback(sender, app_data, user_data):
    i = user_data
    is_auto = dpg.get_value(sender)
    if dpg.does_item_exist(f"p{i}_v_min"):
        dpg.configure_item(f"p{i}_v_min", enabled=not is_auto, readonly=is_auto)
    if dpg.does_item_exist(f"p{i}_v_max"):
        dpg.configure_item(f"p{i}_v_max", enabled=not is_auto, readonly=is_auto)
    _plot_config_changed()

def _rebuild_plot_ui():
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
                    dpg.add_input_text(width=50, default_value=s.get("name", ""), tag=f"p{i}_s{j}_name", callback=_plot_config_changed)
                    dpg.add_text("Unit:")
                    dpg.add_input_text(width=30, default_value=s.get("unit", ""), tag=f"p{i}_s{j}_unit", callback=_plot_config_changed)
                    
                    dpg.add_text("Color:")
                    dpg.add_input_text(width=65, default_value=s.get("color", "#00FF00"), tag=f"p{i}_s{j}_color", callback=_hex_input_changed, user_data=(i, j))
                    dpg.add_color_button(default_value=_hex_to_rgba(s.get("color", "#00FF00")), tag=f"p{i}_s{j}_color_box", no_alpha=True, no_tooltip=True)
                    
                    with dpg.popup(f"p{i}_s{j}_color_box", mousebutton=dpg.mvMouseButton_Left):
                        dpg.add_text("Palette (50 Colors)")
                        for row in range(10):
                            with dpg.group(horizontal=True):
                                for col in range(5):
                                    c_hex = PLOT_PALETTE[row*5 + col]
                                    dpg.add_color_button(default_value=_hex_to_rgba(c_hex), callback=_palette_color_clicked, user_data=(i, j, c_hex), no_alpha=True)
                    
                    dpg.add_button(label="[X]", callback=_delete_series_callback, user_data=(i, j))
                    dpg.bind_item_theme(dpg.last_item(), THEME_RED_BTN)
            
            with dpg.group(horizontal=True):
                dpg.add_spacer(width=20)
                dpg.add_button(label="[+ Add Series]", callback=_add_series_callback, user_data=i)
            
            dpg.add_spacer(height=5)
            dpg.add_text("Vertical Axis (Shared):", color=COLOR_AXIS_LBL)
            
            v_auto_scale = cfg.get("v_auto_scale", False)
            with dpg.group(horizontal=True):
                dpg.add_checkbox(label="Auto-scale", default_value=v_auto_scale, tag=f"p{i}_v_auto_scale", callback=_auto_scale_changed_callback, user_data=i)
                
            with dpg.group(horizontal=True):
                dpg.add_text("Min:")
                dpg.add_input_float(width=150, default_value=cfg["v_min"], tag=f"p{i}_v_min", callback=_plot_config_changed, enabled=not v_auto_scale, readonly=v_auto_scale)
            with dpg.group(horizontal=True):
                dpg.add_text("Max:")
                dpg.add_input_float(width=150, default_value=cfg["v_max"], tag=f"p{i}_v_max", callback=_plot_config_changed, enabled=not v_auto_scale, readonly=v_auto_scale)
                
            dpg.add_text("Horizontal Axis (Time):", color=COLOR_AXIS_LBL)
            with dpg.group(horizontal=True):
                dpg.add_text("Unit:")
                dpg.add_combo(["seconds", "milliseconds", "microseconds"], width=140, default_value=cfg.get("h_unit", "seconds"), tag=f"p{i}_h_unit", callback=_plot_config_changed)
            dpg.add_spacer(height=5)

def _save_setup_callback(sender, app_data, user_data):
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
        
        # Validate Min/Max (replace comma with dot if string, then convert to float)
        try:
            v_min_str = str(cfg["v_min"]).replace(',', '.')
            v_max_str = str(cfg["v_max"]).replace(',', '.')
            v_min = float(v_min_str)
            v_max = float(v_max_str)
            if v_min >= v_max:
                _log_to_console(f"[!] Error (Plot {i}): Vertical Min must be strictly less than Max.")
                return
            cfg["v_min"] = v_min
            cfg["v_max"] = v_max
        except ValueError:
            _log_to_console(f"[!] Error (Plot {i}): Min and Max must be proper numerical values.")
            return

    protocol = dpg.get_value("combo_protocol")
    target = dpg.get_value("combo_target_port") if protocol == "USB (Serial)" else dpg.get_value("input_target")
    setup_data = {
        "protocol": protocol,
        "target": target,
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
        else:
            dpg.set_value("input_target", setup_data.get("target", ""))
            
        dpg.set_value("combo_layout", setup_data.get("layout", "2x3"))
        
        global PLOT_CACHE
        PLOT_CACHE.clear()
        PLOT_CACHE.update(setup_data.get("plots", {}))
        
        _rebuild_plot_ui()
        _log_to_console(f"[*] Loaded config from '{setup_name}.json'")
    except Exception as e:
        _log_to_console(f"[!] Error loading config: {str(e)}")

def _build_section_connection():
    dpg.add_text(">> 1. HARDWARE CONNECTION", color=COLOR_SECTION_HEADING)
    dpg.add_combo(["BLE (ESP32-S3)", "LoRa (Serial)", "USB (Serial)"], default_value=DEFAULT_PROTOCOL, label="Protocol", tag="combo_protocol", callback=_protocol_changed_callback)
    
    ports = get_com_ports() if DEFAULT_PROTOCOL == "USB (Serial)" else []
    dpg.add_combo(ports, default_value=ports[0] if ports else "", label="Target / Port", tag="combo_target_port", show=(DEFAULT_PROTOCOL=="USB (Serial)"))
    
    with dpg.item_handler_registry(tag="combo_click_handler"):
        dpg.add_item_clicked_handler(callback=_rescan_ports_callback)
    dpg.bind_item_handler_registry("combo_target_port", "combo_click_handler")
    
    dpg.add_input_text(default_value=DEFAULT_TARGET, label="Target / Port", tag="input_target", show=(DEFAULT_PROTOCOL!="USB (Serial)"))
    
    with dpg.group(horizontal=True):
        dpg.add_button(label="[CONNECT]", width=140, callback=_connect_callback)
        dpg.bind_item_theme(dpg.last_item(), THEME_GREEN_BTN)
        
        dpg.add_button(label="[TERMINATE]", width=140, callback=_disconnect_callback)
        dpg.bind_item_theme(dpg.last_item(), THEME_RED_BTN)
    dpg.add_spacer(height=20)

def _build_section_layout():
    dpg.add_text(">> 2. DATA VISUALIZATION", color=COLOR_SECTION_HEADING)
    dpg.add_combo(["1x1", "1x2", "1x3", "2x2", "2x3", "3x3"], default_value=DEFAULT_LAYOUT, label="Grid Layout", tag="combo_layout", callback=_apply_layout_callback)
    dpg.add_spacer(height=5)
    dpg.add_group(tag="plot_settings_container")
    dpg.add_spacer(height=20)

def _listbox_double_clicked(sender, app_data, user_data):
    selected = dpg.get_value("list_available_setups")
    if selected:
        dpg.set_value("input_setup_name", selected)

def _delete_setup_callback(sender, app_data, user_data):
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
    from core.state_manager import set_config_dir, get_available_layouts
    folder_path = app_data['file_path_name']
    set_config_dir(folder_path)
    dpg.set_value("text_save_folder", f"Folder: {folder_path}")
    available_setups = get_available_layouts()
    dpg.configure_item("list_available_setups", items=available_setups)
    _log_to_console(f"[*] Save folder changed to: {folder_path}")

def _build_section_session():
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
    folder_path = app_data['file_path_name']
    dpg.set_value("text_data_folder", f"Folder: {folder_path}")
    _log_to_console(f"[*] Data capture folder changed to: {folder_path}")

def _build_section_additional_settings():
    dpg.add_text(">> 4. ADDITIONAL SETTINGS", color=COLOR_SECTION_HEADING)
    
    with dpg.file_dialog(directory_selector=True, show=False, callback=_data_folder_picker_callback, tag="data_folder_picker_dialog", width=500, height=400):
        pass

    dpg.add_checkbox(label="Save Captured Data to Disk", default_value=False, tag="checkbox_save_data")

    with dpg.group(horizontal=True):
        dpg.add_text("Folder: default (/captured_data)", tag="text_data_folder", wrap=260)
        dpg.add_button(label="[Select]", width=100, callback=lambda: dpg.show_item("data_folder_picker_dialog"))
        
    current_time_str = datetime.datetime.now().strftime("FCDatMon %Y-%m-%d_%H-%M")
    dpg.add_input_text(default_value=current_time_str, label="File Name", tag="input_data_filename", width=250)
    
    dpg.add_spacer(height=20)


def _build_console():
    dpg.add_text(">> SYSTEM LOG", color=COLOR_LOG)
    dpg.add_input_text(multiline=True, default_value="[OK] FCDatMon Initialized.", width=-1, height=100, readonly=True, tag="console_output")

def create_control_panel():
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

