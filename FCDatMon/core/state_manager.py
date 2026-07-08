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

# Path to store paired BLE device info
PAIRED_DEVICES_DIR = settings.get("paired_dir", os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "data", "paired_devices"))

def set_paired_dir(new_path: str):
    """
    Updates the global directory where paired BLE device configs are saved.
    """
    global PAIRED_DEVICES_DIR
    PAIRED_DEVICES_DIR = new_path
    
    settings = load_app_settings()
    settings["paired_dir"] = new_path
    save_app_settings(settings)

import re

def _get_mac_filename(device_identifier: str) -> str:
    """Extracts MAC from 'Name (MAC)' or just 'MAC' and returns a valid JSON filename."""
    match = re.search(r'\(([^)]+)\)', device_identifier)
    mac = match.group(1) if match else device_identifier
    return mac.replace(":", "-") + ".json"

def get_paired_ble_devices() -> list[str]:
    """
    Returns a list of paired BLE device names by scanning the PAIRED_DEVICES_DIR.
    """
    devices = set()
    if os.path.exists(PAIRED_DEVICES_DIR):
        for f in os.listdir(PAIRED_DEVICES_DIR):
            if f.endswith(".json"):
                file_path = os.path.join(PAIRED_DEVICES_DIR, f)
                try:
                    with open(file_path, "r", encoding="utf-8") as file:
                        data = json.load(file)
                        mac = data.get("mac", "")
                        name = data.get("name", "Unknown")
                        if name and name != "Unknown" and name != mac:
                            devices.add(f"{name} ({mac})")
                        else:
                            devices.add(mac)
                except:
                    devices.add(f[:-5].replace("-", ":"))
    return sorted(list(devices))

def load_paired_ble_device(device_name: str) -> dict | None:
    """
    Loads BLE device pairing data from paired_devices/{device_name}.json.
    """
    file_path = os.path.join(PAIRED_DEVICES_DIR, _get_mac_filename(device_name))
    if not os.path.exists(file_path):
        return None
        
    with open(file_path, "r", encoding="utf-8") as f:
        return json.load(f)

def save_paired_ble_device(device_name: str, config_data: dict):
    """
    Saves the BLE device pairing data to paired_devices/{device_name}.json.
    """
    os.makedirs(PAIRED_DEVICES_DIR, exist_ok=True)
    file_path = os.path.join(PAIRED_DEVICES_DIR, _get_mac_filename(device_name))
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(config_data, f, indent=4)

def delete_paired_ble_device(device_name: str) -> bool:
    """
    Deletes the BLE device pairing data from paired_devices/{device_name}.json.
    """
    file_path = os.path.join(PAIRED_DEVICES_DIR, _get_mac_filename(device_name))
    if os.path.exists(file_path):
        os.remove(file_path)
        return True
    return False

