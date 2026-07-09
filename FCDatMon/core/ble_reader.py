import threading
import time
import asyncio
import struct
import collections
from bleak import BleakClient, BleakScanner
from core.data_manager import DATA_MANAGER


class BLEReader:
    """
    Background worker that connects to an ESP32 via BLE, subscribes to its
    telemetry notifications, decodes raw 36-byte (9 floats) binary payloads, 
    and pushes them to DATA_MANAGER.
    """
    def __init__(self, mac, service_uuid, char_uuid, log_file_path=None, on_disconnect=None, on_connect_success=None, on_connect_error=None, timestamp_mode="RX", timestamp_unit="milliseconds", batch_size=1, log_frequency=False, freq_window_size=500, on_freq_log=None):
        self.mac = mac
        self.service_uuid = service_uuid
        self.char_uuid = char_uuid
        self.log_file_path = log_file_path
        self.log_file = None
        self.on_freq_log = on_freq_log
        
        self.timestamp_mode = timestamp_mode
        self.timestamp_unit = timestamp_unit
        self.batch_size = batch_size
        
        self.on_disconnect = on_disconnect
        self.on_connect_success = on_connect_success
        self.on_connect_error = on_connect_error
        
        self.client = None
        self.thread = None
        self.is_running = False
        self.is_streaming = False
        self.start_time = 0
        self.loop = None
        
        self.log_frequency = log_frequency
        self.freq_window_size = freq_window_size
        self.dt_window = collections.deque(maxlen=self.freq_window_size)
        self.dt_sum = 0.0
        self.last_freq_log_time = 0
        self.prev_t = None
        
        self.stop_event = threading.Event()

    def start(self):
        """Spawns the background asyncio loop thread for Bleak."""
        if self.is_running:
            return True, "Already running"
            
        try:
            if self.log_file_path:
                import os
                os.makedirs(os.path.dirname(self.log_file_path), exist_ok=True)
                self.log_file = open(self.log_file_path, "a")
                
            self.stop_event.clear()
            DATA_MANAGER.clear()
            self.thread = threading.Thread(target=self._run_async_loop, daemon=True)
            self.thread.start()
            
            self.is_running = True
            self.is_streaming = False
            self.dt_window.clear()
            self.dt_sum = 0.0
            self.prev_t = None
            self.last_freq_log_time = time.perf_counter()
            return True, "BLE Background thread started"
        except Exception as e:
            return False, str(e)

    def pause(self):
        self.is_streaming = False

    def resume(self):
        self.is_streaming = True

    def stop(self):
        """Signals the reading thread to stop and wait for it."""
        self.is_running = False
        self.stop_event.set()
        
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=3.0)
            
        if self.log_file:
            try:
                self.log_file.close()
            except Exception:
                pass
            self.log_file = None

    def _run_async_loop(self):
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        try:
            self.loop.run_until_complete(self._connect_and_read())
        finally:
            self.loop.close()

    async def _connect_and_read(self):
        has_connected = False

        def _disconnected_callback(client):
            self.is_running = False
            self.stop_event.set()
            if not has_connected:
                if self.on_connect_error: self.on_connect_error("Disconnected during pairing")
            else:
                if self.on_disconnect: self.on_disconnect()

        def _notification_handler(sender, data: bytearray):
            if not self.is_running or not self.is_streaming:
                return
            
            data_len = len(data)
            if data_len == 0 or data_len % self.batch_size != 0:
                return
                
            sample_size = data_len // self.batch_size
            if sample_size % 4 != 0:
                return
                
            curr_time = time.perf_counter()
                
            for b in range(self.batch_size):
                sample_data = data[b * sample_size : (b + 1) * sample_size]
                try:
                    if self.timestamp_mode == "TX":
                        num_floats = (sample_size - 4) // 4
                        unpacked = struct.unpack(f"<I{num_floats}f", sample_data)
                        raw_t = unpacked[0]
                        vals = unpacked[1:]
                        
                        if self.timestamp_unit == "microseconds":
                            t = raw_t / 1000.0
                        else:
                            t = float(raw_t)
                    else: # RX
                        num_floats = sample_size // 4
                        vals = struct.unpack(f"<{num_floats}f", sample_data)
                        
                        t = (curr_time - self.start_time) * 1000.0
                        if self.timestamp_unit == "microseconds":
                            t *= 1000.0
                            
                    if self.log_frequency:
                        if self.prev_t is not None and t > self.prev_t:
                            dt = t - self.prev_t
                            if len(self.dt_window) == self.dt_window.maxlen:
                                self.dt_sum -= self.dt_window.popleft()
                            self.dt_window.append(dt)
                            self.dt_sum += dt
                        self.prev_t = t

                        if curr_time - self.last_freq_log_time >= 2.5:
                            if len(self.dt_window) > 0 and self.dt_sum > 0:
                                avg_dt = self.dt_sum / len(self.dt_window)
                                freq = 1000.0 / avg_dt
                                msg = f"BLE Packet Frequency: {freq:.2f} Hz (Window: {len(self.dt_window)})"
                                if self.on_freq_log:
                                    self.on_freq_log(msg)
                                else:
                                    print(msg)
                            self.last_freq_log_time = curr_time
                        
                    DATA_MANAGER.push_data(t, list(vals))
                    
                    if self.log_file:
                        csv_str = f"{t:.3f}," + ",".join(f"{v:.4f}" for v in vals) + "\n"
                        self.log_file.write(csv_str)
                except Exception:
                    pass

        try:
            device = await BleakScanner.find_device_by_address(self.mac, timeout=5.0)
            target = device if device else self.mac
            
            async with BleakClient(target, timeout=20.0, disconnected_callback=_disconnected_callback) as self.client:
                if not self.client.is_connected:
                    self.is_running = False
                    if self.on_connect_error: self.on_connect_error("Could not connect to device.")
                    return
                
                self.start_time = time.perf_counter()
                await self.client.start_notify(self.char_uuid, _notification_handler)
                
                has_connected = True
                if self.on_connect_success:
                    self.on_connect_success()
                
                # Keep alive
                while not self.stop_event.is_set() and self.client.is_connected:
                    await asyncio.sleep(0.1)
                
                if self.client.is_connected:
                    try:
                        await self.client.stop_notify(self.char_uuid)
                    except Exception:
                        pass # Ignore benign teardown errors
                    
        except Exception as e:
            self.is_running = False
            if not has_connected:
                if self.on_connect_error:
                    self.on_connect_error(f"Error connecting: {str(e)}")
            else:
                if self.on_disconnect:
                    self.on_disconnect()
