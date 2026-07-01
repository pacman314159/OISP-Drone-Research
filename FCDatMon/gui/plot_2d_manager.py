import dearpygui.dearpygui as dpg

PLOT_WINDOW_TAG = "PlotManager_Window"
SUBPLOT_TAG = "PlotManager_Subplots"
PLOT_THEME_TAG = "theme_light_plots"

def _hex_to_rgba(hex_str):
    """
    Safely parses a hexadecimal color string (e.g. '#FF0000') into an RGBA integer tuple (255, 0, 0, 255)
    used internally by DearPyGui for rendering.
    """
    hex_str = str(hex_str).strip().lstrip('#')
    if len(hex_str) != 6:
        return (0, 0, 0, 255)
    try:
        return (int(hex_str[0:2], 16), int(hex_str[2:4], 16), int(hex_str[4:6], 16), 255)
    except ValueError:
        return (0, 0, 0, 255)

def _setup_plot_theme():
    """
    Registers the global plot color themes for backgrounds, grids, axis texts, and titles.
    Configures a clean, bright aesthetic for the logic analyzer UI.
    """
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
    """
    Initializes the primary window container on the right side of the screen where
    all telemetry subplots will be rendered.
    """
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
            title = cfg.get("title", f"Plot {i}")
            
            with dpg.plot(label=title, no_title=False, tag=f"plot_{i}"):
                # Add Legend
                dpg.add_plot_legend()
                
                # Setup X Axis
                dpg.add_plot_axis(dpg.mvXAxis, label="Time", tag=f"plot_{i}_x")
                
                # Setup Y Axis
                series_list = cfg.get("series", [])
                unique_units = list(dict.fromkeys([s.get("unit", "") for s in series_list if s.get("unit")]))
                y_unit_str = f" ({', '.join(unique_units)})" if unique_units else ""
                y_axis_label = f"Values{y_unit_str}"
                
                dpg.add_plot_axis(dpg.mvYAxis, label=y_axis_label, tag=f"plot_{i}_y")
                dpg.set_axis_limits_auto(f"plot_{i}_y")
                
                for j, s in enumerate(series_list):
                    s_name = s.get("name", f"Series {j+1}")
                    line_color_hex = s.get("color", "#000000")
                    try:
                        s_weight = float(s.get("width", 2.0))
                    except (ValueError, TypeError):
                        s_weight = 2.0
                    
                    series_tag = f"plot_{i}_series_{j+1}"
                    dpg.add_line_series([], [], label=s_name, tag=series_tag, parent=f"plot_{i}_y")
                    
                    with dpg.theme() as series_theme:
                        with dpg.theme_component(dpg.mvLineSeries):
                            dpg.add_theme_color(dpg.mvPlotCol_Line, _hex_to_rgba(line_color_hex), category=dpg.mvThemeCat_Plots)
                            dpg.add_theme_style(dpg.mvPlotStyleVar_LineWeight, s_weight, category=dpg.mvThemeCat_Plots)
                    dpg.bind_item_theme(series_tag, series_theme)
