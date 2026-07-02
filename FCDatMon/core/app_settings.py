import os
import json

SETTINGS_FILE = os.path.join(os.path.expanduser("~"), ".fcdatmon", "app_settings.json")
DEFAULT_SETTINGS = {
    "config_dir": os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "saved_setups"),
    "data_dir": os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "captured_data")
}

def load_app_settings():
    if os.path.exists(SETTINGS_FILE):
        try:
            with open(SETTINGS_FILE, "r", encoding="utf-8") as f:
                settings = json.load(f)
                # merge with defaults
                for k, v in DEFAULT_SETTINGS.items():
                    if k not in settings:
                        settings[k] = v
                return settings
        except Exception:
            pass
    return DEFAULT_SETTINGS.copy()

def save_app_settings(settings):
    os.makedirs(os.path.dirname(SETTINGS_FILE), exist_ok=True)
    with open(SETTINGS_FILE, "w", encoding="utf-8") as f:
        json.dump(settings, f, indent=4)
