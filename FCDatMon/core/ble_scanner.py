import asyncio
import threading
from bleak import BleakScanner

def run_ble_scan(callback, timeout=5.0):
    """
    Runs a BLE scan in a background thread so the GUI does not freeze.
    Calls `callback(devices_list)` when done.
    """
    def _scan_thread():
        async def _scan():
            try:
                # bleak's discover returns a list of BLEDevice objects
                devices = await BleakScanner.discover(timeout=timeout)
                return devices
            except Exception as e:
                print(f"[BLE ERROR] Scanner failed: {e}")
                return []
                
        devices = asyncio.run(_scan())
        
        # Format the devices for the UI: "Name (MAC)", prioritizing known names
        known_devices = []
        unknown_devices = []
        
        for d in devices:
            if d.name and d.name != "Unknown":
                known_devices.append(f"{d.name} ({d.address})")
            else:
                unknown_devices.append(f"Unknown ({d.address})")
                
        known_devices.sort()
        unknown_devices.sort()
        device_strings = known_devices + unknown_devices
            
        # Fire callback with the formatted list
        callback(device_strings)
        
    threading.Thread(target=_scan_thread, daemon=True).start()
