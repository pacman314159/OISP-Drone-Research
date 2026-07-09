import threading
import collections

class TelemetryDataManager:
    """
    Thread-safe data manager that buffers incoming telemetry data in memory using deques.
    Handles dynamic column generation and provides the GUI with continuous access to the buffer.
    """
    def __init__(self, maxlen=10000):
        self.maxlen = maxlen
        self.lock = threading.Lock()
        self.time_data = collections.deque(maxlen=maxlen)
        # Dictionary mapping column_index (int) -> deque of values
        self.series_data = {}
        
    def push_data(self, t: float, vals: list[float]):
        """
        Push a new time point and a list of values.
        vals[0] will be stored in column index 0, vals[1] in 1, etc.
        """
        with self.lock:
            self.time_data.append(t)
            for i, v in enumerate(vals):
                if i not in self.series_data:
                    self.series_data[i] = collections.deque(maxlen=self.maxlen)
                    # Pad new series with zeros or previous length to keep alignment
                    if len(self.time_data) > 1:
                        self.series_data[i].extend([0.0] * (len(self.time_data) - 1))
                self.series_data[i].append(v)
                
    def get_data(self):
        """
        Returns a tuple (time_list, dict_of_series_lists)
        This creates a copy so DearPyGui can safely read it outside the lock.
        """
        with self.lock:
            t = list(self.time_data)
            y = {k: list(v) for k, v in self.series_data.items()}
            return t, y
            
    def clear(self):
        """
        Wipes all buffered telemetry data across all series.
        Used when the user explicitly clicks [CLEAR] or when a new connection is established.
        """
        with self.lock:
            self.time_data.clear()
            self.series_data.clear()

    def resize_buffer(self, new_maxlen):
        """
        Dynamically resizes the maximum capacity of the data queues.
        Trims existing data if the new size is smaller than the current length.
        """
        with self.lock:
            self.maxlen = new_maxlen
            self.time_data = collections.deque(self.time_data, maxlen=new_maxlen)
            for i in self.series_data:
                self.series_data[i] = collections.deque(self.series_data[i], maxlen=new_maxlen)

# Global singleton
DATA_MANAGER = TelemetryDataManager(maxlen=10000)
