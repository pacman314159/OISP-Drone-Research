import datetime
import os
import dearpygui.dearpygui as dpg

from core.app_settings import load_app_settings
from core.state_manager import get_available_layouts
import gui.ui_state as state
import gui.ui_callbacks as cb

def _setup_themes():
    with dpg.theme(tag="theme_folder_picker"):
        with dpg.theme_component(dpg.mvWindowAppItem):
            dpg.add_theme_color(dpg.mvThemeCol_WindowBg, (30, 30, 30, 255))
            dpg.add_theme_style(dpg.mvStyleVar_WindowPadding, 15, 15)
            
    with dpg.theme(tag=state.THEME_RED_BTN):
        with dpg.theme_component(dpg.mvButton):
            dpg.add_theme_color(dpg.mvThemeCol_Text, (255, 80, 80))

    with dpg.theme(tag=state.THEME_GREEN_BTN):
        with dpg.theme_component(dpg.mvButton):
            dpg.add_theme_color(dpg.mvThemeCol_Text, (80, 255, 80))
            
    with dpg.theme(tag=state.THEME_YELLOW_BTN):
        with dpg.theme_component(dpg.mvButton):
            dpg.add_theme_color(dpg.mvThemeCol_Text, (255, 255, 80))

def _build_section_connection():
    """Constructs the '1. PROTOCOL' section of the control panel."""
    dpg.add_text(">> 1. PROTOCOL", color=state.COLOR_SECTION_HEADING)
    dpg.add_combo(["BLE", "USB (Serial)"], default_value=state.DEFAULT_PROTOCOL, label="Protocol", tag="combo_protocol", callback=cb._protocol_changed_callback)
    
    # USB Group
    with dpg.group(tag="group_usb", show=(state.DEFAULT_PROTOCOL=="USB (Serial)")):
        ports = cb.get_com_ports() if state.DEFAULT_PROTOCOL == "USB (Serial)" else []
        dpg.add_combo(ports, default_value=ports[0] if ports else "", label="Target / Port", tag="combo_target_port")
        with dpg.item_handler_registry(tag="combo_click_handler"):
            dpg.add_item_clicked_handler(callback=cb._rescan_ports_callback)
        dpg.bind_item_handler_registry("combo_target_port", "combo_click_handler")
        dpg.add_combo(["9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"], default_value="115200", label="Baud Rate", tag="combo_baudrate")
        
    # BLE Group
    with dpg.group(tag="group_ble", show=(state.DEFAULT_PROTOCOL=="BLE")):
        dpg.add_text("Scan pairing", color=(100, 255, 100))
        with dpg.group(horizontal=True):
            with dpg.group():
                dpg.add_button(label="[SCAN]", tag="btn_scan_ble", width=180, callback=cb._scan_ble_callback)
                dpg.add_listbox([], tag="list_scanned_devices", width=180, num_items=5, callback=cb._ble_listbox_clicked, user_data="single")
            dpg.add_spacer(width=10)
            with dpg.group():
                with dpg.group(horizontal=True):
                    dpg.add_spacer(width=25)
                    dpg.add_text("PAIRED DEVICES", color=state.COLOR_AXIS_LBL)
                dpg.add_listbox([], tag="list_paired_devices", width=180, num_items=5, callback=cb._ble_listbox_clicked, user_data="single", default_value="")
                
        with dpg.item_handler_registry(tag="list_scanned_double_click_handler"):
            dpg.add_item_double_clicked_handler(callback=cb._ble_listbox_clicked, user_data=("double", "list_scanned_devices"))
        dpg.bind_item_handler_registry("list_scanned_devices", "list_scanned_double_click_handler")

        with dpg.item_handler_registry(tag="list_paired_double_click_handler"):
            dpg.add_item_double_clicked_handler(callback=cb._ble_listbox_clicked, user_data=("double", "list_paired_devices"))
        dpg.bind_item_handler_registry("list_paired_devices", "list_paired_double_click_handler")
        dpg.add_spacer(height=10)
        dpg.add_text("Manual pairing", color=(100, 255, 100))
        dpg.add_input_text(default_value="", label=" Target name", tag="input_ble_target", callback=cb._eval_pair_btn_state)
        dpg.add_input_text(default_value="", label=" Service UUID", tag="input_ble_service", callback=cb._eval_pair_btn_state)
        dpg.add_input_text(default_value="", label=" Char UUID", tag="input_ble_char", callback=cb._eval_pair_btn_state)
        
        dpg.add_spacer(height=5)
        with dpg.group(horizontal=True):
            dpg.add_button(label="[PAIR]", tag="btn_pair_ble", width=110, callback=cb._pair_ble_callback)
            dpg.add_button(label="[UNPAIR]", tag="btn_unpair_ble", width=110, callback=cb._unpair_ble_callback)
            dpg.add_button(label="[FORGET]", tag="btn_forget_ble", width=110, callback=cb._forget_device_callback)

    # Fallback Group
    dpg.add_input_text(default_value=state.DEFAULT_TARGET, label="Target / Port", tag="input_target", show=(state.DEFAULT_PROTOCOL!="USB (Serial)" and state.DEFAULT_PROTOCOL!="BLE"))
        
    cb.update_ui_state(state.UIState.INIT)
    dpg.add_spacer(height=20)

def _build_section_payload_settings():
    """Constructs the '2. PAYLOAD SETTINGS' section."""
    dpg.add_text(">> 2. PAYLOAD SETTINGS", color=state.COLOR_SECTION_HEADING)
    with dpg.group(tag="group_payload_settings", show=(state.DEFAULT_PROTOCOL in ["BLE", "USB (Serial)"])):
        dpg.add_combo(["RX", "TX"], default_value="RX", label="Timestamp", tag="combo_timestamp", callback=cb._update_expected_format_callback)
        dpg.add_combo(["milliseconds", "microseconds"], default_value="milliseconds", label="Time Unit", tag="combo_timestamp_unit")
        dpg.add_input_int(default_value=1, min_value=1, label="Batch size", tag="input_batch_size", width=120, show=(state.DEFAULT_PROTOCOL=="BLE"))
        dpg.add_spacer(height=5)
        dpg.add_text("Expected Payload Format:", color=(200, 200, 200))
        dpg.add_text("Calculating...", color=(150, 255, 150), tag="text_expected_format")
    dpg.add_text("Payload settings unsupported for this protocol.", color=state.COLOR_AXIS_LBL, show=(state.DEFAULT_PROTOCOL not in ["BLE", "USB (Serial)"]), tag="text_payload_unsupported")
    dpg.add_spacer(height=10)
    
    with dpg.group(horizontal=True):
        dpg.add_button(label="[CONNECT]", width=110, callback=cb._connect_callback, tag="btn_connect")
        dpg.add_button(label="[TERMINATE]", width=110, callback=cb._disconnect_callback, tag="btn_terminate")
        dpg.add_button(label="[CLEAR]", width=110, callback=cb._clear_callback, tag="btn_clear")
        
    dpg.add_spacer(height=20)

def _build_section_layout():
    """Constructs the '3. DATA VISUALIZATION' section."""
    dpg.add_text(">> 3. DATA VISUALIZATION", color=state.COLOR_SECTION_HEADING)
    with dpg.group(horizontal=True):
        dpg.add_button(label="[CHASE STREAM]", width=120, tag="btn_global_chase", callback=cb._toggle_chase_callback, user_data="global")
        if all(state.PLOT_CHASE_ACTIVE.get(i, True) for i in state.PLOT_CACHE.keys()):
            dpg.bind_item_theme("btn_global_chase", state.THEME_GREEN_BTN)
            
        dpg.add_button(label="[FIT VERTICAL]", width=120, tag="btn_global_fit_y", callback=cb._fit_y_callback, user_data="global")
        dpg.add_button(label="1S", width=30, tag="btn_global_1s", callback=cb._time_window_callback, user_data=("global", 1000.0))
        dpg.add_button(label="2S", width=30, tag="btn_global_2s", callback=cb._time_window_callback, user_data=("global", 2000.0))
        dpg.add_button(label="5S", width=30, tag="btn_global_5s", callback=cb._time_window_callback, user_data=("global", 5000.0))
        dpg.add_button(label="10S", width=35, tag="btn_global_10s", callback=cb._time_window_callback, user_data=("global", 10000.0))
    
    dpg.add_spacer(height=5)
    dpg.add_combo(["1x1", "1x2", "2x1", "2x2", "2x3", "3x1", "3x2", "3x3"], default_value="1x2", label="Grid Layout", tag="combo_layout", callback=cb._layout_changed_callback)
    dpg.add_spacer(height=5)
    dpg.add_group(tag="plot_settings_container")
    dpg.add_spacer(height=20)

def _build_section_session():
    """Constructs the '4. SESSION STATE' section."""
    dpg.add_text(">> 4. SESSION STATE", color=state.COLOR_SECTION_HEADING)
    
    with dpg.file_dialog(directory_selector=True, show=False, callback=cb._handle_folder_selection, user_data="setup", tag="folder_picker_dialog", width=500, height=400): pass
    dpg.add_input_text(default_value=state.DEFAULT_SETUP_NAME, label="Setup Name", tag="input_setup_name")
    
    with dpg.group(horizontal=True):
        dpg.add_button(label="[SAVE]", width=120, callback=cb._manage_setup_callback, user_data="save")
        dpg.add_button(label="[LOAD]", width=120, callback=cb._manage_setup_callback, user_data="load")
        
    dpg.add_text("Available Setups:", color=state.COLOR_AXIS_LBL)
    settings = load_app_settings()
    with dpg.group(horizontal=True):
        dpg.add_text(f"Folder: {settings['config_dir']}", tag="text_save_folder", wrap=250)
        dpg.add_button(label="[Select]", width=120, callback=lambda: dpg.show_item("folder_picker_dialog"))
    
    with dpg.group(horizontal=True):
        dpg.add_listbox(get_available_layouts(), width=240, num_items=8, tag="list_available_setups")
        dpg.add_button(label="[DELETE]", width=80, callback=cb._manage_setup_callback, user_data="delete")
        dpg.bind_item_theme(dpg.last_item(), state.THEME_RED_BTN)
        
    with dpg.item_handler_registry(tag="listbox_double_click_handler"):
        dpg.add_item_double_clicked_handler(callback=cb._listbox_double_clicked)
    dpg.bind_item_handler_registry("list_available_setups", "listbox_double_click_handler")
    dpg.add_spacer(height=20)

def _build_section_additional_settings():
    """Constructs the '5. ADDITIONAL SETTINGS' section."""
    dpg.add_text(">> 5. ADDITIONAL SETTINGS", color=state.COLOR_SECTION_HEADING)
    
    with dpg.file_dialog(directory_selector=True, show=False, callback=cb._handle_folder_selection, user_data="data", tag="data_folder_picker_dialog", width=500, height=400): pass
    with dpg.file_dialog(directory_selector=True, show=False, callback=cb._handle_folder_selection, user_data="paired", tag="paired_folder_picker_dialog", width=500, height=400): pass

    with dpg.group(horizontal=True):
        dpg.add_text("Sample Buffer Size:")
        dpg.add_input_int(default_value=10000, tag="input_buffer_size", callback=cb._buffer_size_changed, width=120)
        
    dpg.add_spacer(height=5)
    dpg.add_checkbox(label="Packet Frequency Log", default_value=state.LOG_FREQUENCY, tag="checkbox_freq_log", callback=cb._freq_log_toggled)
    with dpg.group(horizontal=True):
        dpg.add_text("Window Size:")
        dpg.add_input_int(default_value=state.FREQUENCY_WINDOW_SIZE, min_value=10, tag="input_freq_window", callback=cb._freq_window_changed, width=100)
        
    dpg.add_spacer(height=5)
    settings = load_app_settings()
    with dpg.group(horizontal=True):
        dpg.add_text("BLE Scan Timeout (s): ")
        dpg.add_input_float(default_value=settings.get("ble_scan_timeout", 5.0), tag="input_ble_scan_timeout", callback=cb._ble_timeout_changed_callback, width=120)
        
    dpg.add_spacer(height=5)
    dpg.add_checkbox(label="Save Captured Data to Disk", default_value=False, tag="checkbox_save_data")

    with dpg.group(horizontal=True):
        dpg.add_text(f"Capture Data: {settings.get('data_dir', '')}", tag="text_data_folder", wrap=260)
        dpg.add_button(label="[Select]", width=100, callback=lambda: dpg.show_item("data_folder_picker_dialog"))

    with dpg.group(horizontal=True):
        paired_dir = settings.get("paired_dir", "")
        if not paired_dir:
            paired_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "data", "paired_devices")
        dpg.add_text(f"Paired Devices: {paired_dir}", tag="text_paired_folder", wrap=260)
        dpg.add_button(label="[Select]", width=100, callback=lambda: dpg.show_item("paired_folder_picker_dialog"))
        
    current_time_str = datetime.datetime.now().strftime("FCDatMon_%Y-%m-%d_%H-%M-%S")
    dpg.add_input_text(default_value=current_time_str, label="File Name", tag="input_data_filename", width=250)
    dpg.add_spacer(height=20)

def _build_console():
    """Constructs the system log console text area."""
    dpg.add_text(">> SYSTEM LOG", color=state.COLOR_LOG)
    with dpg.group(horizontal=True):
        dpg.add_checkbox(label="Auto-scroll", default_value=True, tag="checkbox_autoscroll_log")
        dpg.add_spacer(width=5)
        dpg.add_button(label="Clear", callback=cb._clear_console_callback)
    with dpg.child_window(width=-1, height=180, tag="console_window"):
        dpg.add_text("[OK] FCDatMon Initialized.", tag="console_output_text")

def create_control_panel():
    """Master function that initializes the entire Control Panel window."""
    _setup_themes()
    with dpg.window(label="Control Panel", tag="ControlPanel_Window", width=state.WINDOW_WIDTH, height=970, pos=[0,0], no_close=True, no_move=True, no_collapse=True, min_size=[state.WINDOW_WIDTH, 200], max_size=[state.WINDOW_WIDTH, 970]):
        with dpg.group(horizontal=True):
            dpg.add_text("FCDatMon v1.1", color=state.COLOR_TITLE)
            dpg.add_spacer(width=200)
            dpg.add_button(label="[<< Hide]", tag="btn_hide_panel")
        dpg.add_text("-" * 45, color=state.COLOR_TITLE)
        
        _build_section_connection()
        _build_section_payload_settings()
        _build_section_layout()
        _build_section_session()
        _build_section_additional_settings()
        _build_console()
        
    cb._apply_layout_callback()

# For backward compatibility with main.py which directly accesses these through control_panel
PLOT_CACHE = state.PLOT_CACHE
PLOT_CHASE_ACTIVE = state.PLOT_CHASE_ACTIVE
