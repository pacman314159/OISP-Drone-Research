import os
import sys
import dearpygui.dearpygui as dpg

from gui.control_panel import create_control_panel
from gui.plot_2d_manager import create_plot_window

def resource_path(relative_path):
    """ Get absolute path to resource, works for dev and for PyInstaller """
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

def _toggle_panel(sender=None, app_data=None, user_data=None):
    if dpg.is_item_shown("ControlPanel_Window"):
        dpg.hide_item("ControlPanel_Window")
        dpg.show_item("RestorePanel_Window")
    else:
        dpg.show_item("ControlPanel_Window")
        dpg.hide_item("RestorePanel_Window")
    
    if dpg.is_viewport_ok():
        w = dpg.get_viewport_client_width()
        h = dpg.get_viewport_client_height()
        _global_resize_callback(None, [w, h])

def _global_resize_callback(sender, app_data):
    w = app_data[0]
    h = app_data[1]
    
    if dpg.does_item_exist("PlotManager_Window"):
        padding_right = 15
        if dpg.is_item_shown("ControlPanel_Window"):
            plot_x = 420
            new_w = max(100, w - 420 - padding_right)
        else:
            plot_x = 0
            new_w = max(100, w - padding_right)
            
        dpg.configure_item("PlotManager_Window", pos=[plot_x, 0], width=new_w, height=h)

def main():
    """
    FCDatMon Application Entry Point.
    Initializes the GUI context, creates the docking space, and launches the main loop.
    """
    dpg.create_context()
    with dpg.font_registry():
        font_path = resource_path("JetBrainsMonoNerdFont-SemiBold.ttf")
        if os.path.exists(font_path):
            custom_font = dpg.add_font(font_path, 18)
        else:
            # Fallback to standard Windows font if custom font isn't found
            custom_font = dpg.add_font("C:/Windows/Fonts/consola.ttf", 15)
    
    dpg.bind_font(custom_font)

    try:
        import ctypes
        user32 = ctypes.windll.user32
        screen_width = user32.GetSystemMetrics(0)
        screen_height = user32.GetSystemMetrics(1)
        x_pos = max(0, (screen_width - 1820) // 2)
        y_pos = max(0, (screen_height - 1020) // 2)
    except Exception:
        x_pos = 50
        y_pos = 50

    dpg.create_viewport(title="FCDatMon - Aerospace Telemetry Visualizer", width=1820, height=1020, x_pos=x_pos, y_pos=y_pos)
    dpg.setup_dearpygui()
    dpg.set_viewport_resize_callback(_global_resize_callback)

    create_plot_window()
    create_control_panel()
    
    with dpg.window(tag="RestorePanel_Window", pos=[10, 10], no_title_bar=True, no_resize=True, no_move=True, no_background=True, show=False):
        dpg.add_button(label=">>", width=30, height=30, callback=_toggle_panel)
        
    dpg.set_item_callback("btn_hide_panel", _toggle_panel)
    
    dpg.show_viewport()
    dpg.start_dearpygui()
    dpg.destroy_context()

if __name__ == "__main__":
    main()
