import threading
import serial
import time
import collections
from core.data_manager import DATA_MANAGER

class SerialReader:
    """
    Background worker that continuously reads and decodes incoming serial CSV data.
    Runs on an isolated thread to prevent blocking the GUI render loop.
    """
    def __init__(self, port, baudrate=115200, log_file_path=None, timestamp_mode="RX", timestamp_unit="milliseconds", log_frequency=False, freq_window_size=500, on_freq_log=None):
        self.port = port
        self.baudrate = baudrate
        self.log_file_path = log_file_path
        self.on_freq_log = on_freq_log
        self.timestamp_mode = timestamp_mode
        self.timestamp_unit = timestamp_unit
        self.log_frequency = log_frequency
        self.freq_window_size = freq_window_size
        self.log_file = None
        self.serial_port = None
        self.thread = None
        self.is_running = False
        self.is_streaming = False
        self.start_time = 0
        
        self.dt_window = collections.deque(maxlen=self.freq_window_size)
        self.dt_sum = 0.0
        self.last_freq_log_time = 0
        self.prev_t = None
        
    def start(self):
        """
        Attempts to open the serial port and spawns the background reading thread.
        Records the start_time to calculate true receiver-side telemetry timestamps.
        """
        if self.is_running:
            return True, "Already running"
            
        try:
            if self.log_file_path:
                import os
                os.makedirs(os.path.dirname(self.log_file_path), exist_ok=True)
                self.log_file = open(self.log_file_path, "a")
                
            self.serial_port = serial.Serial(self.port, self.baudrate, timeout=1.0)
            self.is_running = True
            self.is_streaming = True
            self.start_time = time.perf_counter()
            self.dt_window.clear()
            self.dt_sum = 0.0
            self.prev_t = None
            self.last_freq_log_time = time.perf_counter()
            
            DATA_MANAGER.clear()
            self.thread = threading.Thread(target=self._read_loop, daemon=True)
            self.thread.start()
            return True, "Connected successfully"
        except Exception as e:
            return False, str(e)
            
    def pause(self):
        """
        Pauses data ingestion but keeps the Serial connection alive.
        """
        self.is_streaming = False

    def resume(self):
        """
        Resumes data ingestion without clearing the old data buffer.
        """
        self.is_streaming = True

    def stop(self):
        """
        Signals the reading thread to stop, waits for it to join, and safely closes the port.
        """
        self.is_running = False
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=2.0)
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
        if self.log_file:
            try:
                self.log_file.close()
            except Exception:
                pass
            self.log_file = None
            
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
                if not decoded or not self.is_streaming:
                    continue
                    
                parts = [p.strip() for p in decoded.split(',')]
                if len(parts) >= 1:
                    try:
                        vals_raw = [float(p) for p in parts if p]
                        if not vals_raw: continue
                        
                        if self.timestamp_mode == "TX":
                            raw_t = vals_raw[0]
                            vals = vals_raw[1:]
                            if self.timestamp_unit == "microseconds":
                                t = raw_t / 1000.0
                            else:
                                t = raw_t
                        else:
                            t = (time.perf_counter() - self.start_time) * 1000.0
                            vals = vals_raw
                            
                        if self.log_frequency:
                            curr_time = time.perf_counter()
                            if self.prev_t is not None and t > self.prev_t:
                                dt = t - self.prev_t
                                if len(self.dt_window) == self.dt_window.maxlen:
                                    self.dt_sum -= self.dt_window.popleft()
                                self.dt_window.append(dt)
                                self.dt_sum += dt
                            self.prev_t = t

                            if curr_time - self.last_freq_log_time >= 5.0:
                                if len(self.dt_window) > 0 and self.dt_sum > 0:
                                    avg_dt = self.dt_sum / len(self.dt_window)
                                    freq = 1000.0 / avg_dt
                                    msg = f"USB Packet Frequency: {freq:.2f} Hz (Window: {len(self.dt_window)})"
                                    if self.on_freq_log:
                                        self.on_freq_log(msg)
                                    else:
                                        print(msg)
                                self.last_freq_log_time = curr_time
                                
                        if vals:
                            DATA_MANAGER.push_data(t, vals)
                            if self.log_file:
                                csv_str = f"{t:.3f}," + ",".join(str(v) for v in vals) + "\n"
                                self.log_file.write(csv_str)
                                self.log_file.flush()
                    except ValueError:
                        pass
            except Exception:
                self.is_running = False
                break
