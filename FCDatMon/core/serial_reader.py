import threading
import serial
import time
from core.data_manager import DATA_MANAGER

class SerialReader:
    """
    Background worker that continuously reads and decodes incoming serial CSV data.
    Runs on an isolated thread to prevent blocking the GUI render loop.
    """
    def __init__(self, port, baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_port = None
        self.thread = None
        self.is_running = False
        self.start_time = 0
        
    def start(self):
        """
        Attempts to open the serial port and spawns the background reading thread.
        Records the start_time to calculate true receiver-side telemetry timestamps.
        """
        if self.is_running:
            return True, "Already running"
            
        try:
            self.serial_port = serial.Serial(self.port, self.baudrate, timeout=1.0)
            self.is_running = True
            self.start_time = time.perf_counter()
            DATA_MANAGER.clear()
            self.thread = threading.Thread(target=self._read_loop, daemon=True)
            self.thread.start()
            return True, "Connected successfully"
        except Exception as e:
            return False, str(e)
            
    def stop(self):
        """
        Signals the reading thread to stop, waits for it to join, and safely closes the port.
        """
        self.is_running = False
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=2.0)
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            
    def _read_loop(self):
        """
        Infinite loop running on the background thread.
        Reads lines, splits CSV data, calculates high-precision elapsed time in ms,
        and pushes the payload to the TelemetryDataManager.
        """
        while self.is_running and self.serial_port and self.serial_port.is_open:
            try:
                line = self.serial_port.readline()
                if not line:
                    continue
                    
                decoded = line.decode('utf-8', errors='ignore').strip()
                if not decoded:
                    continue
                    
                parts = [p.strip() for p in decoded.split(',')]
                if len(parts) >= 1:
                    try:
                        # Time is now calculated by receiver in milliseconds
                        t = (time.perf_counter() - self.start_time) * 1000.0
                        vals = [float(p) for p in parts if p]
                        if vals:
                            DATA_MANAGER.push_data(t, vals)
                    except ValueError:
                        pass
            except Exception:
                self.is_running = False
                break
