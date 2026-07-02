import os
import json

from core.app_settings import load_app_settings, save_app_settings

settings = load_app_settings()
CONFIG_DIR = settings["config_dir"]

def set_config_dir(new_path: str):
    """
    Updates the global configuration directory where setup files are saved and loaded.
    """
    global CONFIG_DIR
    CONFIG_DIR = new_path
    
    settings = load_app_settings()
    settings["config_dir"] = new_path
    save_app_settings(settings)

def save_layout(setup_name: str, config_data: dict) -> str:
    """
    Saves the layout configuration data to configs/{setup_name}.json.
    Returns the path to the saved file.
    """
    os.makedirs(CONFIG_DIR, exist_ok=True)
    file_path = os.path.join(CONFIG_DIR, f"{setup_name}.json")
    
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(config_data, f, indent=4)
        
    return file_path

def load_layout(setup_name: str) -> dict | None:
    """
    Loads layout configuration data from configs/{setup_name}.json.
    Converts plots dictionary keys from string to integer.
    Returns the config dictionary, or None if the file is not found.
    """
    file_path = os.path.join(CONFIG_DIR, f"{setup_name}.json")
    if not os.path.exists(file_path):
        return None
        
    with open(file_path, "r", encoding="utf-8") as f:
        config_data = json.load(f)
        
    # Convert stringified plot keys to integers
    if "plots" in config_data and isinstance(config_data["plots"], dict):
        plots = {}
        for k, v in config_data["plots"].items():
            try:
                plots[int(k)] = v
            except ValueError:
                plots[k] = v
        config_data["plots"] = plots
        
    return config_data

def get_available_layouts() -> list[str]:
    """
    Returns a list of available setup names by scanning the CONFIG_DIR.
    """
    setups = set()
    if os.path.exists(CONFIG_DIR):
        for f in os.listdir(CONFIG_DIR):
            if f.endswith(".json") and f != "layouts.json":
                setups.add(f[:-5])
    return sorted(list(setups))

def delete_layout(setup_name: str) -> bool:
    """
    Deletes the layout configuration data from configs/{setup_name}.json.
    Returns True if deleted, False if not found.
    """
    file_path = os.path.join(CONFIG_DIR, f"{setup_name}.json")
    if os.path.exists(file_path):
        os.remove(file_path)
        return True
    return False

