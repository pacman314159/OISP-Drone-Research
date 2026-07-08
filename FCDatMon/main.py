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
            
        dpg.set_item_pos("PlotManager_Window", [plot_x, 0])
        dpg.set_item_width("PlotManager_Window", new_w)
        dpg.set_item_height("PlotManager_Window", max(100, h - 20))
            
def _global_scroll_handler(sender, app_data, user_data):
    """
    Global mouse wheel scroll interceptor for scaling X window while chasing.
    """
    import dearpygui.dearpygui as dpg
    import gui.ui_state as state
    is_shift = dpg.is_key_down(dpg.mvKey_LShift) or dpg.is_key_down(dpg.mvKey_RShift)
    
    # Get global mouse Y position to detect if we are hovering the bottom area (X-axis)
    mouse_y = dpg.get_mouse_pos(local=False)[1]
        
    for i in list(state.PLOT_CACHE.keys()):
        if dpg.does_item_exist(f"plot_{i}") and dpg.is_item_hovered(f"plot_{i}"):
            rect_max_y = dpg.get_item_rect_max(f"plot_{i}")[1]
            
            # The X-axis is roughly the bottom 45 pixels of the plot bounding box
            is_hovering_x_axis = (mouse_y >= rect_max_y - 45)
            
            # Allow zooming X if Shift is pressed OR if explicitly hovering the X-axis
            if is_shift or is_hovering_x_axis:
                if state.PLOT_CHASE_ACTIVE.get(i, True):
                    limits = dpg.get_axis_limits(f"plot_{i}_x")
                    window = limits[1] - limits[0]
                    if window <= 0: window = 10000.0
                    factor = 1.15 if app_data < 0 else 0.85
                    setattr(state, f"MANUAL_WINDOW_{i}", window * factor)

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
    import gui.ui_state as state
    import gui.plot_state_machine as psm
    
    while dpg.is_dearpygui_running():
        # Execute deferred UI events
        while getattr(state, "UI_EVENT_QUEUE", []):
            event = state.UI_EVENT_QUEUE.pop(0)
            event()
            
        # Fetch live data
        t_data, y_series = DATA_MANAGER.get_data()
        
        is_hw_connected = state.CURRENT_READER is not None
        is_live = is_hw_connected and getattr(state.CURRENT_READER, 'is_streaming', False)
        
        was_live = getattr(state, "WAS_LIVE", False)
        if was_live and not is_live:
            # Connection just stopped! Unlock all axes so the user can pan freely.
            for i in list(state.PLOT_CACHE.keys()):
                if dpg.does_item_exist(f"plot_{i}_x"):
                    dpg.set_axis_limits_auto(f"plot_{i}_x")
                if dpg.does_item_exist(f"plot_{i}_y"):
                    dpg.set_axis_limits_auto(f"plot_{i}_y")
                    
        state.WAS_LIVE = is_live
        
        # If we have data, distribute it to the plots
        if t_data:
            col_idx = 0
            for i in sorted(list(state.PLOT_CACHE.keys())):
                cfg = state.PLOT_CACHE[i]
                series_list = cfg.get("series", [])
                for j in range(len(series_list)):
                    series_tag = f"plot_{i}_series_{j+1}"
                    if dpg.does_item_exist(series_tag):
                        if col_idx in y_series:
                            min_len = min(len(t_data), len(y_series[col_idx]))
                            dpg.set_value(series_tag, [t_data[:min_len], y_series[col_idx][:min_len]])
                    col_idx += 1
        
        # Smart State Machine and Limit Constraints
        for i in list(state.PLOT_CACHE.keys()):
            if dpg.does_item_exist(f"plot_{i}_x") and dpg.does_item_exist(f"plot_{i}_y"):
                is_chasing = state.PLOT_CHASE_ACTIVE.get(i, True)
                is_y_fixed = state.PLOT_CACHE[i].get("fix_y", False)
                
                state_val = psm.get_current_state(is_chasing, is_live, is_hw_connected, is_y_fixed)
                v_pan, h_pan, v_zoom, h_zoom = psm.get_gesture_allowance(state_val)
                
                last_allowances = getattr(state, f"last_allowances_{i}", (True, True, True, True))
                last_v_pan, last_h_pan, last_v_zoom, last_h_zoom = last_allowances
                
                # Explicitly unlock axes if a restricted gesture becomes unrestricted
                if (not last_v_pan and v_pan) or (not last_v_zoom and v_zoom):
                    dpg.set_axis_limits_auto(f"plot_{i}_y")
                if (not last_h_pan and h_pan) or (not last_h_zoom and h_zoom):
                    dpg.set_axis_limits_auto(f"plot_{i}_x")
                
                setattr(state, f"last_allowances_{i}", (v_pan, h_pan, v_zoom, h_zoom))
                
                x_limits = list(dpg.get_axis_limits(f"plot_{i}_x"))
                y_limits = list(dpg.get_axis_limits(f"plot_{i}_y"))
                last_x = getattr(state, f"last_x_limits_{i}", x_limits)
                last_y = getattr(state, f"last_y_limits_{i}", y_limits)
                
                # Check for forced Y-snap events
                if state.PLOT_CACHE[i].get("trigger_y_snap", False):
                    state.PLOT_CACHE[i]["trigger_y_snap"] = False
                    if is_y_fixed:
                        try:
                            y_min = float(state.PLOT_CACHE[i]["y_min"])
                            y_max = float(state.PLOT_CACHE[i]["y_max"])
                            y_limits = [y_min, y_max]
                            dpg.set_axis_limits(f"plot_{i}_y", y_min, y_max)
                        except ValueError:
                            pass
                else:
                    # Mathematically enforce Y pan/zoom allowances
                    if not v_pan or not v_zoom:
                        new_center = (y_limits[0] + y_limits[1]) / 2.0
                        old_center = (last_y[0] + last_y[1]) / 2.0
                        new_win = y_limits[1] - y_limits[0]
                        old_win = last_y[1] - last_y[0]
                        
                        target_center = old_center if not v_pan else new_center
                        target_win = old_win if not v_zoom else new_win
                        
                        y_limits = [target_center - target_win/2.0, target_center + target_win/2.0]
                        dpg.set_axis_limits(f"plot_{i}_y", y_limits[0], y_limits[1])

                # Process X logic and Chasing
                if is_live and t_data and is_chasing:
                    t_max = t_data[-1]
                    manual_win = getattr(state, f"MANUAL_WINDOW_{i}", None)
                    if manual_win is not None:
                        target_win = manual_win
                        delattr(state, f"MANUAL_WINDOW_{i}")
                    else:
                        new_win = x_limits[1] - x_limits[0]
                        old_win = last_x[1] - last_x[0]
                        target_win = new_win if h_zoom else old_win
                    
                    if target_win <= 0: target_win = 10000.0
                    
                    x_limits = [t_max - target_win, t_max]
                    dpg.set_axis_limits(f"plot_{i}_x", x_limits[0], x_limits[1])
                else:
                    # Enforce independent X pan/zoom
                    if not h_pan or not h_zoom:
                        new_center = (x_limits[0] + x_limits[1]) / 2.0
                        old_center = (last_x[0] + last_x[1]) / 2.0
                        new_win = x_limits[1] - x_limits[0]
                        old_win = last_x[1] - last_x[0]
                        
                        target_center = old_center if not h_pan else new_center
                        target_win = old_win if not h_zoom else new_win
                        
                        if target_win <= 0: target_win = 10000.0
                        
                        x_limits = [target_center - target_win/2.0, target_center + target_win/2.0]
                        dpg.set_axis_limits(f"plot_{i}_x", x_limits[0], x_limits[1])

                setattr(state, f"last_x_limits_{i}", x_limits)
                setattr(state, f"last_y_limits_{i}", y_limits)

        dpg.render_dearpygui_frame()
        
    dpg.destroy_context()

if __name__ == "__main__":
    main()
