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
    """
    Toggles the visibility of the control panel window and updates the plot manager layout.
    """
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
    """
    Callback triggered on main viewport resize.
    Calculates the remaining screen width and dynamically resizes the Plot Manager window 
    so it seamlessly fills the space next to the control panel.
    """
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

def _global_scroll_handler(sender, app_data, user_data):
    """
    Global mouse wheel scroll interceptor.
    Listens for Shift + Scroll over a plot specifically to scale its time-viewing window,
    bypassing DearPyGui's native engine lock so the user can zoom while the plot chases live data.
    """
    import dearpygui.dearpygui as dpg
    import gui.control_panel as cp
    is_shift = dpg.is_key_down(dpg.mvKey_LShift) or dpg.is_key_down(dpg.mvKey_RShift)
    if not is_shift:
        return
        
    for i in list(cp.PLOT_CACHE.keys()):
        if dpg.does_item_exist(f"plot_{i}") and dpg.is_item_hovered(f"plot_{i}"):
            if cp.PLOT_CHASE_ACTIVE.get(i, True):
                limits = dpg.get_axis_limits(f"plot_{i}_x")
                window = limits[1] - limits[0]
                if window <= 0: window = 10000.0
                # scroll up (positive) -> zoom in -> smaller window
                factor = 1.15 if app_data < 0 else 0.85
                setattr(cp, f"MANUAL_WINDOW_{i}", window * factor)

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
    
    with dpg.handler_registry():
        dpg.add_mouse_wheel_handler(callback=_global_scroll_handler)
    
    dpg.show_viewport()
    
    from core.data_manager import DATA_MANAGER
    from gui.control_panel import PLOT_CACHE
    import gui.control_panel as cp
    
    while dpg.is_dearpygui_running():
        # Fetch live data
        t_data, y_series = DATA_MANAGER.get_data()
        is_live = cp.CURRENT_READER is not None and cp.CURRENT_READER.is_running
        was_live = getattr(cp, "WAS_LIVE", False)
        
        # Smart zooming/panning constraints
        is_shift = dpg.is_key_down(dpg.mvKey_LShift) or dpg.is_key_down(dpg.mvKey_RShift)
        is_dragging = dpg.is_mouse_button_down(dpg.mvMouseButton_Left) or dpg.is_mouse_button_down(dpg.mvMouseButton_Middle)
        
        for i in list(PLOT_CACHE.keys()):
            if dpg.does_item_exist(f"plot_{i}_x") and dpg.does_item_exist(f"plot_{i}_y"):
                cfg = PLOT_CACHE[i]
                fix_y = cfg.get("fix_y", False)
                try:
                    y_min = float(cfg.get("y_min", -100.0))
                    y_max = float(cfg.get("y_max", 100.0))
                except Exception:
                    y_min, y_max = -100.0, 100.0

                if fix_y:
                    dpg.set_axis_limits(f"plot_{i}_y", y_min, y_max)
                    if is_dragging:
                        dpg.configure_item(f"plot_{i}_x", lock_min=False, lock_max=False)
                        dpg.configure_item(f"plot_{i}_y", lock_min=True, lock_max=True)
                    else:
                        dpg.configure_item(f"plot_{i}_x", lock_min=True, lock_max=True)
                        dpg.configure_item(f"plot_{i}_y", lock_min=True, lock_max=True)
                else:
                    if is_dragging:
                        dpg.configure_item(f"plot_{i}_x", lock_min=False, lock_max=False)
                        dpg.configure_item(f"plot_{i}_y", lock_min=False, lock_max=False)
                    elif is_shift:
                        dpg.configure_item(f"plot_{i}_x", lock_min=True, lock_max=True)
                        dpg.configure_item(f"plot_{i}_y", lock_min=True, lock_max=True)
                    else:
                        dpg.configure_item(f"plot_{i}_x", lock_min=True, lock_max=True)
                        dpg.configure_item(f"plot_{i}_y", lock_min=False, lock_max=False)
        
        if was_live and not is_live:
            # Connection just stopped! Unlock all axes so the user can pan freely.
            for i in list(PLOT_CACHE.keys()):
                if dpg.does_item_exist(f"plot_{i}_x"):
                    dpg.set_axis_limits_auto(f"plot_{i}_x")
                if dpg.does_item_exist(f"plot_{i}_y"):
                    dpg.set_axis_limits_auto(f"plot_{i}_y")
                    
        cp.WAS_LIVE = is_live
        
        # If we have data, distribute it to the plots
        if t_data:
            col_idx = 0
            # Map columns sequentially across all plots and their respective series
            for i in sorted(list(PLOT_CACHE.keys())):
                cfg = PLOT_CACHE[i]
                series_list = cfg.get("series", [])
                for j in range(len(series_list)):
                    series_tag = f"plot_{i}_series_{j+1}"
                    if dpg.does_item_exist(series_tag):
                        if col_idx in y_series:
                            # Truncate lists to the same length to prevent DPG crash if read while appending
                            min_len = min(len(t_data), len(y_series[col_idx]))
                            dpg.set_value(series_tag, [t_data[:min_len], y_series[col_idx][:min_len]])
                    col_idx += 1
                
                # Smart Chasing Logic
                if is_live and dpg.does_item_exist(f"plot_{i}_x") and t_data:
                    is_chasing = cp.PLOT_CHASE_ACTIVE.get(i, True)
                    t_max = t_data[-1]
                    limits = dpg.get_axis_limits(f"plot_{i}_x")
                    window = limits[1] - limits[0]
                    
                    manual_win = getattr(cp, f"MANUAL_WINDOW_{i}", None)
                    if manual_win is not None:
                        center = (limits[0] + limits[1]) / 2.0
                        window = manual_win
                        delattr(cp, f"MANUAL_WINDOW_{i}")
                        
                        if window < 10.0: window = 10.0
                        if window > 100000000.0: window = 100000000.0
                        
                        # Apply zoom around center if we are not chasing
                        if not is_chasing:
                            dpg.set_axis_limits(f"plot_{i}_x", center - window/2.0, center + window/2.0)
                            limits = [center - window/2.0, center + window/2.0]
                    
                    if window <= 0.0 or window > 100000000.0:
                        window = 10000.0
                        
                    last_limits = getattr(cp, f"last_limits_{i}", None)
                    
                    if is_chasing:
                        if last_limits is not None and manual_win is None:
                            # Detect manual pan left: both min and max decreased by at least 2% of window
                            threshold = window * 0.02
                            if limits[0] < last_limits[0] - threshold and limits[1] < last_limits[1] - threshold:
                                cp.PLOT_CHASE_ACTIVE[i] = False
                                if dpg.does_item_exist(f"btn_local_chase_{i}"):
                                    dpg.bind_item_theme(f"btn_local_chase_{i}", 0)
                                all_on = all(cp.PLOT_CHASE_ACTIVE.get(p, True) for p in PLOT_CACHE.keys())
                                if dpg.does_item_exist("btn_global_chase"):
                                    dpg.bind_item_theme("btn_global_chase", cp.THEME_GREEN_BTN if all_on else 0)
                                    
                        # If still chasing, advance the axis
                        if cp.PLOT_CHASE_ACTIVE.get(i, True):
                            dpg.set_axis_limits(f"plot_{i}_x", t_max - window, t_max)
                            limits = [t_max - window, t_max]
                            
                    setattr(cp, f"last_limits_{i}", limits)
                    
        dpg.render_dearpygui_frame()
        
    dpg.destroy_context()

if __name__ == "__main__":
    main()
