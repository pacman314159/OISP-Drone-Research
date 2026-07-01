import dearpygui.dearpygui as dpg

PLOT_WINDOW_TAG = "PlotManager_Window"
SUBPLOT_TAG = "PlotManager_Subplots"
PLOT_THEME_TAG = "theme_light_plots"

def _hex_to_rgba(hex_str):
    hex_str = str(hex_str).strip().lstrip('#')
    if len(hex_str) != 6:
        return (0, 0, 0, 255)
    try:
        return (int(hex_str[0:2], 16), int(hex_str[2:4], 16), int(hex_str[4:6], 16), 255)
    except ValueError:
        return (0, 0, 0, 255)

def _setup_plot_theme():
    with dpg.theme(tag=PLOT_THEME_TAG):
        with dpg.theme_component(dpg.mvWindowAppItem):
            dpg.add_theme_color(dpg.mvThemeCol_Text, (30, 30, 30, 255), category=dpg.mvThemeCat_Core)
            dpg.add_theme_style(dpg.mvStyleVar_WindowPadding, 20, 20, category=dpg.mvThemeCat_Core)
            
        with dpg.theme_component(dpg.mvPlot):
            dpg.add_theme_color(dpg.mvPlotCol_FrameBg, (230, 230, 230, 255), category=dpg.mvThemeCat_Plots)
            dpg.add_theme_color(dpg.mvPlotCol_PlotBg, (255, 255, 255, 255), category=dpg.mvThemeCat_Plots)
            dpg.add_theme_color(dpg.mvPlotCol_AxisGrid, (200, 200, 200, 255), category=dpg.mvThemeCat_Plots)
            dpg.add_theme_color(dpg.mvPlotCol_AxisText, (30, 30, 30, 255), category=dpg.mvThemeCat_Plots)
            dpg.add_theme_color(dpg.mvPlotCol_TitleText, (10, 10, 10, 255), category=dpg.mvThemeCat_Plots)
            dpg.add_theme_color(dpg.mvPlotCol_InlayText, (30, 30, 30, 255), category=dpg.mvThemeCat_Plots)
            dpg.add_theme_color(dpg.mvPlotCol_LegendText, (255, 255, 255, 255), category=dpg.mvThemeCat_Plots)

def create_plot_window():
    _setup_plot_theme()
    # Width will be updated dynamically, assume 1820 - 420 = 1400 initially
    with dpg.window(tag=PLOT_WINDOW_TAG, pos=[420, 0], width=1400, height=1020, 
                    no_title_bar=True, no_move=True, no_resize=True, no_collapse=True, no_bring_to_front_on_focus=True):
        pass
    
    dpg.bind_item_theme(PLOT_WINDOW_TAG, PLOT_THEME_TAG)

def update_plots(layout_str, plot_cache):
    """
    Called whenever the layout combo changes or an individual plot setting changes.
    Rebuilds the entire subplot grid.
    """
    if not dpg.does_item_exist(PLOT_WINDOW_TAG):
        return

    # Delete existing subplots if they exist
    dpg.delete_item(PLOT_WINDOW_TAG, children_only=True)
    
    try:
        rows, cols = map(int, layout_str.split('x'))
    except Exception:
        rows, cols = 1, 1

    count = rows * cols

    with dpg.subplots(rows, cols, label="Telemetry Grid", width=-1, height=-1, tag=SUBPLOT_TAG, parent=PLOT_WINDOW_TAG):
        for i in range(1, count + 1):
            cfg = plot_cache.get(i, {})
            title = cfg.get("title", f"Plot {i}")
            v_min = cfg.get("v_min", -180.0)
            v_max = cfg.get("v_max", 180.0)
            h_unit = cfg.get("h_unit", "seconds")
            
            with dpg.plot(label=title, no_title=False, tag=f"plot_{i}"):
                # Add Legend
                dpg.add_plot_legend()
                
                # Setup X Axis
                x_axis_label = f"Time ({h_unit})"
                dpg.add_plot_axis(dpg.mvXAxis, label=x_axis_label, tag=f"plot_{i}_x")
                
                # Setup Y Axis
                series_list = cfg.get("series", [])
                unique_units = list(dict.fromkeys([s.get("unit", "") for s in series_list if s.get("unit")]))
                y_unit_str = f" ({', '.join(unique_units)})" if unique_units else ""
                y_axis_label = f"Values{y_unit_str}"
                
                dpg.add_plot_axis(dpg.mvYAxis, label=y_axis_label, tag=f"plot_{i}_y")
                
                v_auto_scale = cfg.get("v_auto_scale", False)
                if not v_auto_scale:
                    dpg.set_axis_limits(f"plot_{i}_y", v_min, v_max)
                
                for j, s in enumerate(series_list):
                    s_name = s.get("name", f"Series {j+1}")
                    line_color_hex = s.get("color", "#000000")
                    
                    dpg.add_line_series([0], [0], label=s_name, parent=f"plot_{i}_y", tag=f"plot_{i}_series_{j+1}")
                    
                    with dpg.theme() as series_theme:
                        with dpg.theme_component(dpg.mvLineSeries):
                            dpg.add_theme_color(dpg.mvPlotCol_Line, _hex_to_rgba(line_color_hex), category=dpg.mvThemeCat_Plots)
                    dpg.bind_item_theme(f"plot_{i}_series_{j+1}", series_theme)
