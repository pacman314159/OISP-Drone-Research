#!/usr/bin/env python3
"""
universal_flight_analyzer.py - Universal Multi-Rate Flight Log Decoder, Trimmer & Analyzer
for FC_v01 / FC_v02 Flight Controllers.

Capabilities:
1. Parses binary flight logs (LOG_xxxx.BIN) according to Note 0018 specification.
2. Performs pre-scan stream discovery and interactive menu selection.
3. Automatically cuts startup and shutdown transients (keeping steady middle flight data).
4. Symmetrically analyzes RAW and FILTERED streams for Accelerometer, Gyroscope, Magnetometer, and Barometer:
     - Histograms & Standard Deviation distributions
     - Discrete Fast Fourier Transform (FFT) noise & vibration spectra
     - Continuous time series telemetry (with overlays where applicable)
     - 3-plane projections (Magnetometer RAW & FILTERED)
     - Rotated Ellipsoid Calibration with Adam optimizer (Magnetometer RAW)
5. Computes distribution statistics (Mean, StDev, Min, Max, Range, Jitter) and spectral FFT peaks.
6. Automatically saves figures to analyzers/img/ using the format:
     YYYY-XXXXX-<short_description>.png
   where:
     - YYYY: Persistent 4-digit ID up front, shared across all images generated in that run,
             dynamically determined from max ID existing in img/ (max + 1, starting at 0000).
     - XXXXX: Up to 5 characters (HIST, FFT, TIME, PROJ, RMSE)
     - <short_description>: Description without spaces (e.g., Accel_raw, Accel_filt, Gyro_raw, Gyro_filt, Mag_raw, Mag_filt).
"""

import sys
import os
import re
import struct
import argparse
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any

if sys.stdout and hasattr(sys.stdout, 'reconfigure'):
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass
if sys.stderr and hasattr(sys.stderr, 'reconfigure'):
    try:
        sys.stderr.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass

import numpy as np
import matplotlib
matplotlib.use('Agg')  # Headless backend for fast file generation
import matplotlib.pyplot as plt
from ellipsoid_fit import fit_rotated_ellipsoid

# =============================================================================
# BINARY FLIGHT LOG PROTOCOL SPECIFICATION (per Note 0018)
# =============================================================================
HEADER_SIZE = 512
HEADER_FORMAT = "<4sH24sIHBII467s"
HEADER_MAGIC = b"FCLG"

PKT_TYPE_IMU_RAW   = 0x01
PKT_TYPE_IMU_FILT  = 0x02
PKT_TYPE_MAG_RAW   = 0x03
PKT_TYPE_MAG_FILT  = 0x04
PKT_TYPE_BARO_RAW  = 0x05
PKT_TYPE_BARO_FILT = 0x06
PKT_TYPE_ATT_QUAT  = 0x07
PKT_TYPE_ATT_EULER = 0x08
PKT_TYPE_POWER     = 0x09

PACKET_SIZES = {
    PKT_TYPE_IMU_RAW:   31,  # ts (4B) + pad (3B) + 3*gyro (12B) + 3*accel (12B) = 31B payload (32B total)
    PKT_TYPE_IMU_FILT:  31,
    PKT_TYPE_MAG_RAW:   19,  # ts (4B) + pad (3B) + 3*mag (12B) = 19B payload (20B total)
    PKT_TYPE_MAG_FILT:  19,
    PKT_TYPE_BARO_RAW:  8,   # ts (4B) + press (4B) = 8B payload (9B total, packed)
    PKT_TYPE_BARO_FILT: 8,
    PKT_TYPE_ATT_QUAT:  23,  # ts (4B) + pad (3B) + 4*quat (16B) = 23B payload (24B total)
    PKT_TYPE_ATT_EULER: 19,  # ts (4B) + pad (3B) + 3*euler (12B) = 19B payload (20B total)
    PKT_TYPE_POWER:     23,  # ts (4B) + pad (3B) + 4*power_fields (16B) = 23B payload (24B total)
}

# Sea level standard atmospheric pressure (Pa) for barometric altitude
P0_SEA_LEVEL_PA = 101325.0

# =============================================================================
# HELPER FUNCTIONS: FILE PATHS & RUN ID DETERMINATION
# =============================================================================
def get_script_dir() -> Path:
    return Path(__file__).resolve().parent

def get_img_dir() -> Path:
    img_dir = get_script_dir() / "img"
    img_dir.mkdir(parents=True, exist_ok=True)
    return img_dir

def get_log_dir() -> Path:
    log_dir = get_script_dir() / "log"
    log_dir.mkdir(parents=True, exist_ok=True)
    return log_dir

def determine_next_run_id(img_dir: Path, log_dir: Optional[Path] = None) -> int:
    """
    Stateless run ID scanner:
    Scans img/ and log/ for files matching '*-(\\d{4})-*.png' or '(\\d{4})-LOG.txt'.
    Returns max(found_ids) + 1, or 0 if none exist.
    """
    new_pattern = re.compile(r"^(?P<id>\d{4})-[A-Za-z0-9]+-[A-Za-z0-9_]+\.png$")
    old_pattern = re.compile(r"^[A-Za-z0-9]+-(?P<id>\d{4})-[A-Za-z0-9_]+\.png$")
    log_pattern = re.compile(r"^(?P<id>\d{4})-LOG\.txt$")
    existing_ids = []

    dirs_to_scan = [img_dir]
    if log_dir is not None:
        dirs_to_scan.append(log_dir)

    for d in dirs_to_scan:
        if d.exists():
            for entry in d.iterdir():
                if entry.is_file():
                    match = new_pattern.match(entry.name) or old_pattern.match(entry.name) or log_pattern.match(entry.name)
                    if match:
                        try:
                            existing_ids.append(int(match.group("id")))
                        except ValueError:
                            pass

    if not existing_ids:
        return 0
    return max(existing_ids) + 1

def find_default_log_file() -> Optional[Path]:
    """Finds the latest .BIN file in binary_black_box_data/ or current directory."""
    candidates: List[Path] = []
    
    data_dir = get_script_dir() / "binary_black_box_data"
    if data_dir.exists():
        candidates.extend(data_dir.glob("*.BIN"))
        candidates.extend(data_dir.glob("*.bin"))

    candidates.extend(get_script_dir().glob("*.BIN"))
    candidates.extend(get_script_dir().glob("*.bin"))

    if not candidates:
        return None

    # Return newest by modification time
    candidates.sort(key=lambda p: p.stat().st_mtime, reverse=True)
    return candidates[0]

def select_file_via_explorer() -> Optional[Path]:
    """
    Opens a native Windows File Explorer dialog to allow the user
    to browse and pick any .BIN flight log file without modifying code.
    """
    try:
        import tkinter as tk
        from tkinter import filedialog
        
        root = tk.Tk()
        root.withdraw()
        root.attributes('-topmost', True)
        
        initial_dir = get_script_dir() / "binary_black_box_data"
        if not initial_dir.exists():
            initial_dir = get_script_dir()
            
        file_path = filedialog.askopenfilename(
            title="Select Flight Log Binary File (.BIN)",
            initialdir=str(initial_dir),
            filetypes=[
                ("Binary Flight Logs (*.BIN, *.bin)", "*.BIN;*.bin"),
                ("All Files (*.*)", "*.*")
            ]
        )
        root.destroy()
        if file_path:
            return Path(file_path)
        return None
    except Exception as e:
        print(f"[!] Could not open GUI file explorer ({e}).")
        return None

class TeeLogger:
    """
    Dual-stream logger that simultaneously sends all terminal stdout/stderr
    content to both the console and YYYY-LOG.txt file(s).
    """
    def __init__(self, log_paths: List[Path]):
        self.terminal = sys.stdout
        self.files = []
        for p in log_paths:
            try:
                p.parent.mkdir(parents=True, exist_ok=True)
                self.files.append(open(p, "w", encoding="utf-8"))
            except Exception as e:
                if self.terminal:
                    self.terminal.write(f"[!] Warning: could not open log file {p}: {e}\n")

    def write(self, message):
        if self.terminal:
            self.terminal.write(message)
        for f in self.files:
            try:
                f.write(message)
                f.flush()
            except Exception:
                pass

    def flush(self):
        if self.terminal:
            self.terminal.flush()
        for f in self.files:
            try:
                f.flush()
            except Exception:
                pass

    def close(self):
        for f in self.files:
            try:
                f.close()
            except Exception:
                pass
        self.files.clear()

# =============================================================================
# 1. BINARY PARSER MODULE
# =============================================================================
class FlightLogData:
    def __init__(self, filepath: Path):
        self.filepath = filepath
        self.session_id: int = 0
        self.board_name: str = ""
        self.boot_ts_us: int = 0
        self.registered_mask: int = 0
        self.limit_policy: str = ""
        self.max_file_size: int = 0
        self.circular_head: int = 0
        
        self.imu_raw: Optional[np.ndarray] = None
        self.imu_filt: Optional[np.ndarray] = None
        self.mag_raw: Optional[np.ndarray] = None
        self.mag_filt: Optional[np.ndarray] = None
        self.baro_raw: Optional[np.ndarray] = None
        self.baro_filt: Optional[np.ndarray] = None
        self.att_quat: Optional[np.ndarray] = None
        self.att_euler: Optional[np.ndarray] = None
        self.power: Optional[np.ndarray] = None

def parse_binary_log(filepath: Path) -> Optional[FlightLogData]:
    if not filepath.exists():
        print(f"[ERROR] Log file not found: {filepath}")
        return None

    log_data = FlightLogData(filepath)

    with open(filepath, "rb") as f:
        header_bytes = f.read(HEADER_SIZE)
        if len(header_bytes) < HEADER_SIZE:
            print(f"[ERROR] File smaller than 512-byte header ({len(header_bytes)} bytes).")
            return None

        magic, session_id, board_raw, boot_ts, mask, limit_pol, max_sz, circ_head, _ = struct.unpack(
            HEADER_FORMAT, header_bytes
        )

        if magic != HEADER_MAGIC:
            print(f"[WARNING] Magic mismatch: got {magic}, expected {HEADER_MAGIC}. Parsing anyway...")

        log_data.session_id = session_id
        log_data.board_name = board_raw.decode("ascii", errors="ignore").rstrip("\x00")
        log_data.boot_ts_us = boot_ts
        log_data.registered_mask = mask
        log_data.limit_policy = "STOP_ON_SIZE" if limit_pol == 0 else "CIRCULAR_OVERWRITE"
        log_data.max_file_size = max_sz
        log_data.circular_head = circ_head

        imu_raw_list = []
        imu_filt_list = []
        mag_raw_list = []
        mag_filt_list = []
        baro_raw_list = []
        baro_filt_list = []
        quat_list = []
        euler_list = []
        power_list = []

        while True:
            tag_byte = f.read(1)
            if not tag_byte:
                break
            tag = tag_byte[0]

            if tag == 0:
                # Padding at sector boundary: fast forward to next 512-byte boundary
                curr_pos = f.tell()
                next_boundary = ((curr_pos + 511) // 512) * 512
                f.seek(next_boundary)
                continue

            if tag not in PACKET_SIZES:
                continue

            payload_len = PACKET_SIZES[tag]
            payload = f.read(payload_len)
            if len(payload) < payload_len:
                break

            if tag == PKT_TYPE_IMU_RAW:
                ts, gx, gy, gz, ax, ay, az = struct.unpack("<I3xffffff", payload)
                imu_raw_list.append((ts, gx, gy, gz, ax, ay, az))
            elif tag == PKT_TYPE_IMU_FILT:
                ts, gx, gy, gz, ax, ay, az = struct.unpack("<I3xffffff", payload)
                imu_filt_list.append((ts, gx, gy, gz, ax, ay, az))
            elif tag == PKT_TYPE_MAG_RAW:
                ts, mx, my, mz = struct.unpack("<I3xfff", payload[:19])
                mag_raw_list.append((ts, mx, my, mz))
            elif tag == PKT_TYPE_MAG_FILT:
                ts, mx, my, mz = struct.unpack("<I3xfff", payload[:19])
                mag_filt_list.append((ts, mx, my, mz))
            elif tag == PKT_TYPE_BARO_RAW:
                ts, press = struct.unpack("<If", payload[:8])
                baro_raw_list.append((ts, press))
            elif tag == PKT_TYPE_BARO_FILT:
                ts, press = struct.unpack("<If", payload[:8])
                baro_filt_list.append((ts, press))
            elif tag == PKT_TYPE_ATT_QUAT:
                ts, qw, qx, qy, qz = struct.unpack("<I3xffff", payload[:23])
                quat_list.append((ts, qw, qx, qy, qz))
            elif tag == PKT_TYPE_ATT_EULER:
                ts, roll, pitch, yaw = struct.unpack("<I3xfff", payload[:19])
                euler_list.append((ts, roll, pitch, yaw))
            elif tag == PKT_TYPE_POWER:
                ts, v_mv, i_ma, p_mw, e_mah = struct.unpack("<I3xffff", payload[:23])
                power_list.append((ts, v_mv, i_ma, p_mw, e_mah))

    if imu_raw_list:
        log_data.imu_raw = np.array(imu_raw_list, dtype=[
            ('ts_us', 'u4'), ('gx', 'f4'), ('gy', 'f4'), ('gz', 'f4'),
            ('ax', 'f4'), ('ay', 'f4'), ('az', 'f4')
        ])
    if imu_filt_list:
        log_data.imu_filt = np.array(imu_filt_list, dtype=[
            ('ts_us', 'u4'), ('gx', 'f4'), ('gy', 'f4'), ('gz', 'f4'),
            ('ax', 'f4'), ('ay', 'f4'), ('az', 'f4')
        ])
    if mag_raw_list:
        log_data.mag_raw = np.array(mag_raw_list, dtype=[
            ('ts_us', 'u4'), ('mx', 'f4'), ('my', 'f4'), ('mz', 'f4')
        ])
    if mag_filt_list:
        log_data.mag_filt = np.array(mag_filt_list, dtype=[
            ('ts_us', 'u4'), ('mx', 'f4'), ('my', 'f4'), ('mz', 'f4')
        ])
    if baro_raw_list:
        log_data.baro_raw = np.array(baro_raw_list, dtype=[
            ('ts_us', 'u4'), ('press', 'f4')
        ])
    if baro_filt_list:
        log_data.baro_filt = np.array(baro_filt_list, dtype=[
            ('ts_us', 'u4'), ('press', 'f4')
        ])
    if quat_list:
        log_data.att_quat = np.array(quat_list, dtype=[
            ('ts_us', 'u4'), ('qw', 'f4'), ('qx', 'f4'), ('qy', 'f4'), ('qz', 'f4')
        ])
    if euler_list:
        log_data.att_euler = np.array(euler_list, dtype=[
            ('ts_us', 'u4'), ('roll', 'f4'), ('pitch', 'f4'), ('yaw', 'f4')
        ])
    if power_list:
        log_data.power = np.array(power_list, dtype=[
            ('ts_us', 'u4'), ('voltage_mv', 'f4'), ('current_ma', 'f4'),
            ('power_mw', 'f4'), ('energy_mah', 'f4')
        ])

    return log_data

# =============================================================================
# 2. DATA TRIMMING & WINDOWING ENGINE
# =============================================================================
def trim_data_stream(
    arr: np.ndarray,
    trim_start_pct: float = 10.0,
    trim_end_pct: float = 10.0
) -> Tuple[np.ndarray, int, int]:
    """
    Cuts the beginning and end of a stream to discard startup/landing transients,
    leaving the valuable steady-state flight data in the middle.
    Returns (trimmed_array, start_idx, end_idx).
    """
    n = len(arr)
    if n <= 10:
        return arr, 0, n

    start_idx = int(np.floor(n * (trim_start_pct / 100.0)))
    end_idx = int(np.ceil(n * (1.0 - (trim_end_pct / 100.0))))

    # Clamp bounds safely
    start_idx = max(0, min(start_idx, n - 2))
    end_idx = max(start_idx + 2, min(end_idx, n))

    return arr[start_idx:end_idx], start_idx, end_idx

# =============================================================================
# 3. TERMINAL STATISTICS REPORTER
# =============================================================================
def calculate_stream_stats(
    name: str,
    ts_us: np.ndarray,
    values: np.ndarray,
    unit: str
) -> Dict[str, Any]:
    n = len(values)
    vals_f64 = values.astype(np.float64)
    mean_val = float(np.mean(vals_f64))
    std_val = float(np.std(vals_f64))
    min_val = float(np.min(vals_f64))
    max_val = float(np.max(vals_f64))
    peak_to_peak = max_val - min_val

    # Timing analysis
    if len(ts_us) >= 2:
        intervals_us = np.diff(ts_us.astype(np.float64))
        # Filter negative or zero jumps if present
        valid_intervals = intervals_us[intervals_us > 0]
        if len(valid_intervals) > 0:
            mean_dt_s = np.mean(valid_intervals) / 1e6
            fs_hz = 1.0 / mean_dt_s if mean_dt_s > 0 else 0.0
            jitter_us = float(np.std(valid_intervals))
            duration_s = (float(ts_us[-1]) - float(ts_us[0])) / 1e6
        else:
            fs_hz = 0.0
            jitter_us = 0.0
            duration_s = 0.0
    else:
        fs_hz = 0.0
        jitter_us = 0.0
        duration_s = 0.0

    # FFT peak detection
    peak_freq_hz = 0.0
    peak_db = -100.0
    if n >= 16 and fs_hz > 0:
        detrended = vals_f64 - mean_val
        fft_vals = np.fft.rfft(detrended)
        freqs = np.fft.rfftfreq(n, d=(1.0 / fs_hz))
        amps = np.abs(fft_vals) * 2.0 / n
        amps_db = 20.0 * np.log10(np.maximum(amps, 1e-10))
        if len(freqs) > 1:
            # Skip 0 Hz DC
            freqs_no_dc = freqs[1:]
            amps_no_dc = amps_db[1:]
            max_idx = np.argmax(amps_no_dc)
            peak_freq_hz = float(freqs_no_dc[max_idx])
            peak_db = float(amps_no_dc[max_idx])

    return {
        "name": name,
        "n": n,
        "unit": unit,
        "mean": mean_val,
        "std": std_val,
        "min": min_val,
        "max": max_val,
        "range": peak_to_peak,
        "duration_s": duration_s,
        "fs_hz": fs_hz,
        "jitter_us": jitter_us,
        "peak_freq_hz": peak_freq_hz,
        "peak_db": peak_db
    }

def print_stats_table(stats_list: List[Dict[str, Any]], title: str):
    print(f"\n==========================================================================================")
    print(f"  STATISTICAL REPORT: {title}")
    print(f"==========================================================================================")
    print(f" {'Channel':<18} | {'Samples':<7} | {'Mean':<11} | {'StDev':<11} | {'Range':<11} | {'Fs (Hz)':<8} | {'Peak Freq':<10}")
    print(f"-------------------+---------+-------------+-------------+-------------+----------+-----------")
    for s in stats_list:
        mean_str = f"{s['mean']:+.4f} {s['unit']}"
        std_str = f"{s['std']:.4f} {s['unit']}"
        range_str = f"{s['range']:.4f}"
        fs_str = f"{s['fs_hz']:.1f}" if s['fs_hz'] > 0 else "N/A"
        peak_str = f"{s['peak_freq_hz']:.1f} Hz" if s['peak_freq_hz'] > 0 else "N/A"
        print(f" {s['name']:<18} | {s['n']:<7} | {mean_str:<11} | {std_str:<11} | {range_str:<11} | {fs_str:<8} | {peak_str:<10}")
    print(f"==========================================================================================\n")

# =============================================================================
# 4. PLOTTING & SAVE ENGINE
# =============================================================================
def save_figure(fig: plt.Figure, img_dir: Path, plot_type: str, run_id: int, short_desc: str) -> Path:
    """
    Saves figure in format: YYYY-XXXXX-<short_description>.png
    - YYYY: 4-digit zero-padded run ID up front
    - XXXXX: up to 5 chars (HIST, FFT)
    - short_desc: strictly alphanumeric and underscores (no spaces)
    """
    clean_type = re.sub(r'[^A-Za-z0-9]', '', plot_type)[:5].upper()
    clean_desc = re.sub(r'\s+', '_', short_desc)
    clean_desc = re.sub(r'[^A-Za-z0-9_]', '', clean_desc)

    filename = f"{run_id:04d}-{clean_type}-{clean_desc}.png"
    out_path = img_dir / filename
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  [OK] Saved: {out_path.name}  -->  {out_path}")
    return out_path

def plot_histogram_3axis(
    ts_us: np.ndarray,
    data_x: np.ndarray,
    data_y: np.ndarray,
    data_z: np.ndarray,
    sensor_name: str,
    unit: str,
    short_desc: str,
    run_id: int,
    img_dir: Path,
    color: str = "#4C72B0"
):
    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    fig.suptitle(f"{sensor_name} Distribution & Standard Deviation (Middle Flight Data)", fontsize=14, fontweight="bold")

    axes_data = [
        ("X Axis", data_x),
        ("Y Axis", data_y),
        ("Z Axis", data_z)
    ]

    for ax, (axis_name, data) in zip(axes, axes_data):
        data_f64 = data.astype(np.float64)
        mean_val = float(np.mean(data_f64))
        std_val = float(np.std(data_f64))

        ax.hist(data_f64, bins=50, alpha=0.75, color=color, edgecolor="black", linewidth=0.5)
        ax.axvline(mean_val, color="red", linestyle="solid", linewidth=2.0, label=f"Mean: {mean_val:+.4f} {unit}")
        ax.axvline(mean_val - std_val, color="orange", linestyle="dashed", linewidth=1.8, label=f"SD: {std_val:.4f} {unit}")
        ax.axvline(mean_val + std_val, color="orange", linestyle="dashed", linewidth=1.8)

        ax.set_title(f"{sensor_name} {axis_name}", fontsize=11, fontweight="bold")
        ax.set_xlabel(f"Value [{unit}]", fontsize=10)
        ax.set_ylabel("Frequency", fontsize=10)
        ax.legend(loc="upper right", fontsize=8.5)
        ax.grid(True, alpha=0.3, linestyle=":")

    plt.tight_layout()
    save_figure(fig, img_dir, "HIST", run_id, short_desc)

def plot_fft_3axis(
    ts_us: np.ndarray,
    data_x: np.ndarray,
    data_y: np.ndarray,
    data_z: np.ndarray,
    sensor_name: str,
    unit: str,
    short_desc: str,
    run_id: int,
    img_dir: Path,
    color: str = "midnightblue"
):
    n = len(data_x)
    if n < 16:
        print(f"  [!] Skipping FFT for {sensor_name}: sample count ({n}) too low.")
        return

    intervals = np.diff(ts_us.astype(np.float64))
    valid_int = intervals[intervals > 0]
    if len(valid_int) == 0:
        print(f"  [!] Skipping FFT for {sensor_name}: invalid timestamps.")
        return

    dt_s = np.mean(valid_int) / 1e6
    fs_hz = 1.0 / dt_s if dt_s > 0 else 1.0

    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    fig.suptitle(f"{sensor_name} Discrete FFT Spectrum (fs ≈ {fs_hz:.1f} Hz)", fontsize=14, fontweight="bold")

    axes_data = [
        ("X Axis", data_x),
        ("Y Axis", data_y),
        ("Z Axis", data_z)
    ]

    for ax, (axis_name, data) in zip(axes, axes_data):
        data_f64 = data.astype(np.float64)
        detrended = data_f64 - np.mean(data_f64)
        fft_vals = np.fft.rfft(detrended)
        freqs = np.fft.rfftfreq(n, d=dt_s)
        amps = np.abs(fft_vals) * 2.0 / n
        amps_db = 20.0 * np.log10(np.maximum(amps, 1e-10))

        if len(freqs) > 1:
            freqs_plot = freqs[1:]
            amps_plot = amps_db[1:]
            ax.plot(freqs_plot, amps_plot, color=color, linewidth=0.8)

            peak_idx = np.argmax(amps_plot)
            peak_f = freqs_plot[peak_idx]
            peak_a = amps_plot[peak_idx]
            ax.axvline(peak_f, color="crimson", linestyle="--", alpha=0.7, label=f"Peak: {peak_f:.1f} Hz ({peak_a:.1f} dB)")
            ax.legend(loc="upper right", fontsize=8.5)

        ax.set_title(f"{sensor_name} {axis_name}", fontsize=11, fontweight="bold")
        ax.set_xlabel("Frequency [Hz]", fontsize=10)
        ax.set_ylabel("Amplitude [dB]", fontsize=10)
        ax.set_xlim(0, fs_hz / 2.0)
        ax.grid(True, alpha=0.3, linestyle=":")

    plt.tight_layout()
    save_figure(fig, img_dir, "FFT", run_id, short_desc)

def plot_baro_histogram(
    ts_us: np.ndarray,
    pressure_pa: np.ndarray,
    short_desc: str,
    run_id: int,
    img_dir: Path,
    sensor_name: str = "Barometer"
):
    # Altitude estimation using hypsometric formula
    altitude_m = 44330.0 * (1.0 - (pressure_pa / P0_SEA_LEVEL_PA) ** (1.0 / 5.255))

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle(f"{sensor_name} Distribution & Standard Deviation (Middle Flight Data)", fontsize=14, fontweight="bold")

    # Pressure
    mean_p = float(np.mean(pressure_pa))
    std_p = float(np.std(pressure_pa))
    ax1.hist(pressure_pa, bins=50, alpha=0.75, color="skyblue", edgecolor="black", linewidth=0.5)
    ax1.axvline(mean_p, color="red", linestyle="solid", linewidth=2.0, label=f"Mean: {mean_p:.1f} Pa")
    ax1.axvline(mean_p - std_p, color="purple", linestyle="dashed", linewidth=1.8, label=f"SD: {std_p:.3f} Pa")
    ax1.axvline(mean_p + std_p, color="purple", linestyle="dashed", linewidth=1.8)
    ax1.set_title("Atmospheric Pressure", fontsize=11, fontweight="bold")
    ax1.set_xlabel("Pressure [Pa]", fontsize=10)
    ax1.set_ylabel("Frequency", fontsize=10)
    ax1.legend(loc="upper right", fontsize=9)
    ax1.grid(True, alpha=0.3, linestyle=":")

    # Altitude
    mean_alt = float(np.mean(altitude_m))
    std_alt = float(np.std(altitude_m))
    ax2.hist(altitude_m, bins=50, alpha=0.75, color="lightgreen", edgecolor="black", linewidth=0.5)
    ax2.axvline(mean_alt, color="red", linestyle="solid", linewidth=2.0, label=f"Mean: {mean_alt:.2f} m")
    ax2.axvline(mean_alt - std_alt, color="purple", linestyle="dashed", linewidth=1.8, label=f"SD: {std_alt:.3f} m")
    ax2.axvline(mean_alt + std_alt, color="purple", linestyle="dashed", linewidth=1.8)
    ax2.set_title("Estimated Altitude (from QNH)", fontsize=11, fontweight="bold")
    ax2.set_xlabel("Altitude [m]", fontsize=10)
    ax2.set_ylabel("Frequency", fontsize=10)
    ax2.legend(loc="upper right", fontsize=9)
    ax2.grid(True, alpha=0.3, linestyle=":")

    plt.tight_layout()
    save_figure(fig, img_dir, "HIST", run_id, short_desc)

def plot_baro_fft(
    ts_us: np.ndarray,
    pressure_pa: np.ndarray,
    short_desc: str,
    run_id: int,
    img_dir: Path,
    sensor_name: str = "Barometer"
):
    n = len(pressure_pa)
    if n < 16:
        print(f"  [!] Skipping FFT for {sensor_name}: sample count ({n}) too low.")
        return

    intervals = np.diff(ts_us.astype(np.float64))
    valid_int = intervals[intervals > 0]
    if len(valid_int) == 0:
        print(f"  [!] Skipping FFT for {sensor_name}: invalid timestamps.")
        return

    dt_s = np.mean(valid_int) / 1e6
    fs_hz = 1.0 / dt_s if dt_s > 0 else 1.0

    fig, ax = plt.subplots(figsize=(10, 5))
    fig.suptitle(f"{sensor_name} Frequency Spectrum (fs ≈ {fs_hz:.1f} Hz)", fontsize=14, fontweight="bold")

    detrended = pressure_pa - np.mean(pressure_pa)
    fft_vals = np.fft.rfft(detrended)
    freqs = np.fft.rfftfreq(n, d=dt_s)
    amps = np.abs(fft_vals) * 2.0 / n
    amps_db = 20.0 * np.log10(np.maximum(amps, 1e-10))

    if len(freqs) > 1:
        freqs_plot = freqs[1:]
        amps_plot = amps_db[1:]
        ax.plot(freqs_plot, amps_plot, color="teal", linewidth=1.0)

        peak_idx = np.argmax(amps_plot)
        peak_f = freqs_plot[peak_idx]
        peak_a = amps_plot[peak_idx]
        ax.axvline(peak_f, color="crimson", linestyle="--", label=f"Peak: {peak_f:.2f} Hz ({peak_a:.1f} dB)")
        ax.legend(loc="upper right", fontsize=9)

    ax.set_title(f"{sensor_name} Pressure Sensor Noise Spectrum", fontsize=11, fontweight="bold")
    ax.set_xlabel("Frequency [Hz]", fontsize=10)
    ax.set_ylabel("Amplitude [dB]", fontsize=10)
    ax.set_xlim(0, fs_hz / 2.0)
    ax.grid(True, alpha=0.3, linestyle=":")

    plt.tight_layout()
    save_figure(fig, img_dir, "FFT", run_id, short_desc)

def plot_time_series_3axis(
    ts_us: np.ndarray,
    data_x: np.ndarray,
    data_y: np.ndarray,
    data_z: np.ndarray,
    sensor_name: str,
    unit: str,
    short_desc: str,
    run_id: int,
    img_dir: Path,
    filt_ts_us: Optional[np.ndarray] = None,
    filt_x: Optional[np.ndarray] = None,
    filt_y: Optional[np.ndarray] = None,
    filt_z: Optional[np.ndarray] = None
):
    n = len(data_x)
    if n == 0:
        return

    time_s = (ts_us.astype(np.float64) - float(ts_us[0])) / 1e6
    filt_time_s = None
    if filt_ts_us is not None and len(filt_ts_us) > 0:
        filt_time_s = (filt_ts_us.astype(np.float64) - float(ts_us[0])) / 1e6

    fig, axes = plt.subplots(3, 1, figsize=(16, 9), sharex=True)
    fig.suptitle(f"{sensor_name} Value vs. Time ({n:,d} samples, {time_s[-1]:.2f} s)", fontsize=14, fontweight="bold")

    channels = [
        ("X Axis", data_x, filt_x, "#1f77b4"),
        ("Y Axis", data_y, filt_y, "#2ca02c"),
        ("Z Axis", data_z, filt_z, "#d62728")
    ]

    for ax, (axis_name, raw_vals, filt_vals, color) in zip(axes, channels):
        if filt_vals is not None and len(filt_vals) > 0 and filt_time_s is not None:
            ax.plot(time_s, raw_vals, color=color, alpha=0.35, linewidth=0.8, label=f"{axis_name} Raw")
            ax.plot(filt_time_s, filt_vals, color=color, linewidth=1.5, label=f"{axis_name} Filtered")
        else:
            ax.plot(time_s, raw_vals, color=color, linewidth=1.0, label=f"{axis_name}")

        mean_val = float(np.mean(raw_vals))
        ax.axhline(mean_val, color="black", linestyle="--", linewidth=1.0, alpha=0.7, label=f"Mean: {mean_val:+.4f} {unit}")
        ax.set_ylabel(f"{axis_name} [{unit}]", fontsize=10, fontweight="bold")
        ax.grid(True, alpha=0.3, linestyle=":")
        ax.legend(loc="upper right", fontsize=8.5)

    axes[-1].set_xlabel("Elapsed Flight Time [seconds]", fontsize=11, fontweight="bold")
    plt.tight_layout()
    save_figure(fig, img_dir, "TIME", run_id, short_desc)

def plot_baro_time_series(
    ts_us: np.ndarray,
    pressure_pa: np.ndarray,
    short_desc: str,
    run_id: int,
    img_dir: Path,
    filt_ts_us: Optional[np.ndarray] = None,
    filt_pressure_pa: Optional[np.ndarray] = None,
    sensor_name: str = "Barometer"
):
    n = len(pressure_pa)
    if n == 0:
        return

    time_s = (ts_us.astype(np.float64) - float(ts_us[0])) / 1e6
    alt_m = 44330.0 * (1.0 - (pressure_pa / P0_SEA_LEVEL_PA) ** (1.0 / 5.255))

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(16, 7), sharex=True)
    fig.suptitle(f"{sensor_name} Time Series ({n:,d} samples, {time_s[-1]:.2f} s)", fontsize=14, fontweight="bold")

    # Pressure
    ax1.plot(time_s, pressure_pa, color="teal", linewidth=1.0, label="Pressure")
    if filt_pressure_pa is not None and filt_ts_us is not None:
        filt_time_s = (filt_ts_us.astype(np.float64) - float(ts_us[0])) / 1e6
        ax1.plot(filt_time_s, filt_pressure_pa, color="darkcyan", linewidth=1.5, label="Filtered Pressure")
    mean_p = float(np.mean(pressure_pa))
    ax1.axhline(mean_p, color="red", linestyle="--", linewidth=1.0, alpha=0.7, label=f"Mean: {mean_p:.1f} Pa")
    ax1.set_ylabel("Pressure [Pa]", fontsize=10, fontweight="bold")
    ax1.grid(True, alpha=0.3, linestyle=":")
    ax1.legend(loc="upper right", fontsize=8.5)

    # Altitude
    ax2.plot(time_s, alt_m, color="forestgreen", linewidth=1.0, label="Estimated Altitude")
    if filt_pressure_pa is not None and filt_ts_us is not None:
        filt_time_s = (filt_ts_us.astype(np.float64) - float(ts_us[0])) / 1e6
        filt_alt_m = 44330.0 * (1.0 - (filt_pressure_pa / P0_SEA_LEVEL_PA) ** (1.0 / 5.255))
        ax2.plot(filt_time_s, filt_alt_m, color="darkgreen", linewidth=1.5, label="Filtered Altitude")
    mean_alt = float(np.mean(alt_m))
    ax2.axhline(mean_alt, color="red", linestyle="--", linewidth=1.0, alpha=0.7, label=f"Mean: {mean_alt:.2f} m")
    ax2.set_ylabel("Altitude [m]", fontsize=10, fontweight="bold")
    ax2.set_xlabel("Elapsed Flight Time [seconds]", fontsize=11, fontweight="bold")
    ax2.grid(True, alpha=0.3, linestyle=":")
    ax2.legend(loc="upper right", fontsize=8.5)

    plt.tight_layout()
    save_figure(fig, img_dir, "TIME", run_id, short_desc)

def plot_attitude_time_series(
    ts_us: np.ndarray,
    roll_deg: np.ndarray,
    pitch_deg: np.ndarray,
    yaw_deg: np.ndarray,
    short_desc: str,
    run_id: int,
    img_dir: Path
):
    n = len(roll_deg)
    if n == 0:
        return

    time_s = (ts_us.astype(np.float64) - float(ts_us[0])) / 1e6
    fig, axes = plt.subplots(3, 1, figsize=(16, 9), sharex=True)
    fig.suptitle(f"Attitude Orientation vs. Time ({n:,d} samples, {time_s[-1]:.2f} s)", fontsize=14, fontweight="bold")

    angles = [
        ("Roll", roll_deg, "#1f77b4"),
        ("Pitch", pitch_deg, "#2ca02c"),
        ("Yaw", yaw_deg, "#d62728")
    ]

    for ax, (name, data, color) in zip(axes, angles):
        ax.plot(time_s, data, color=color, linewidth=1.2, label=name)
        mean_v = float(np.mean(data))
        ax.axhline(mean_v, color="black", linestyle="--", linewidth=1.0, alpha=0.7, label=f"Mean: {mean_v:+.2f}°")
        ax.set_ylabel(f"{name} [deg]", fontsize=10, fontweight="bold")
        ax.grid(True, alpha=0.3, linestyle=":")
        ax.legend(loc="upper right", fontsize=8.5)

    axes[-1].set_xlabel("Elapsed Flight Time [seconds]", fontsize=11, fontweight="bold")
    plt.tight_layout()
    save_figure(fig, img_dir, "TIME", run_id, short_desc)

def plot_power_histogram(
    v_mv: np.ndarray,
    i_ma: np.ndarray,
    p_mw: np.ndarray,
    short_desc: str,
    run_id: int,
    img_dir: Path
):
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(16, 5))
    fig.suptitle("Power Monitor Distribution & Standard Deviation (Middle Flight Data)", fontsize=14, fontweight="bold")

    # 1. Voltage
    v_mean = float(np.mean(v_mv))
    v_std = float(np.std(v_mv))
    ax1.hist(v_mv, bins=50, alpha=0.75, color="#e74c3c", edgecolor="black", linewidth=0.5)
    ax1.axvline(v_mean, color="red", linestyle="solid", linewidth=2.0, label=f"Mean: {v_mean:.1f} mV")
    ax1.axvline(v_mean - v_std, color="darkred", linestyle="dashed", linewidth=1.8, label=f"SD: {v_std:.1f} mV")
    ax1.axvline(v_mean + v_std, color="darkred", linestyle="dashed", linewidth=1.8)
    ax1.set_title("Pack Voltage", fontsize=11, fontweight="bold")
    ax1.set_xlabel("Voltage [mV]", fontsize=10)
    ax1.set_ylabel("Frequency", fontsize=10)
    ax1.legend(loc="upper right", fontsize=8.5)
    ax1.grid(True, alpha=0.3, linestyle=":")

    # 2. Current
    i_mean = float(np.mean(i_ma))
    i_std = float(np.std(i_ma))
    ax2.hist(i_ma, bins=50, alpha=0.75, color="#3498db", edgecolor="black", linewidth=0.5)
    ax2.axvline(i_mean, color="blue", linestyle="solid", linewidth=2.0, label=f"Mean: {i_mean:.1f} mA")
    ax2.axvline(i_mean - i_std, color="darkblue", linestyle="dashed", linewidth=1.8, label=f"SD: {i_std:.1f} mA")
    ax2.axvline(i_mean + i_std, color="darkblue", linestyle="dashed", linewidth=1.8)
    ax2.set_title("Current Draw", fontsize=11, fontweight="bold")
    ax2.set_xlabel("Current [mA]", fontsize=10)
    ax2.set_ylabel("Frequency", fontsize=10)
    ax2.legend(loc="upper right", fontsize=8.5)
    ax2.grid(True, alpha=0.3, linestyle=":")

    # 3. Power
    p_mean = float(np.mean(p_mw))
    p_std = float(np.std(p_mw))
    ax3.hist(p_mw, bins=50, alpha=0.75, color="#f39c12", edgecolor="black", linewidth=0.5)
    ax3.axvline(p_mean, color="darkorange", linestyle="solid", linewidth=2.0, label=f"Mean: {p_mean:.1f} mW")
    ax3.axvline(p_mean - p_std, color="brown", linestyle="dashed", linewidth=1.8, label=f"SD: {p_std:.1f} mW")
    ax3.axvline(p_mean + p_std, color="brown", linestyle="dashed", linewidth=1.8)
    ax3.set_title("Instantaneous Power", fontsize=11, fontweight="bold")
    ax3.set_xlabel("Power [mW]", fontsize=10)
    ax3.set_ylabel("Frequency", fontsize=10)
    ax3.legend(loc="upper right", fontsize=8.5)
    ax3.grid(True, alpha=0.3, linestyle=":")

    plt.tight_layout()
    save_figure(fig, img_dir, "HIST", run_id, short_desc)

def plot_power_time_series(
    ts_us: np.ndarray,
    v_mv: np.ndarray,
    i_ma: np.ndarray,
    p_mw: np.ndarray,
    e_mah: np.ndarray,
    short_desc: str,
    run_id: int,
    img_dir: Path
):
    n = len(v_mv)
    if n == 0:
        return

    time_s = (ts_us.astype(np.float64) - float(ts_us[0])) / 1e6
    fig, axes = plt.subplots(4, 1, figsize=(16, 11), sharex=True)
    fig.suptitle(f"Power & Energy Consumption vs. Time ({n:,d} samples, {time_s[-1]:.2f} s)", fontsize=14, fontweight="bold")

    plots = [
        ("Pack Voltage", v_mv, "mV", "#e74c3c"),
        ("Current Draw", i_ma, "mA", "#3498db"),
        ("Power Dissipation", p_mw, "mW", "#f39c12"),
        ("Cumulative Energy", e_mah, "mAh", "#2ecc71")
    ]

    for ax, (name, data, unit, color) in zip(axes, plots):
        ax.plot(time_s, data, color=color, linewidth=1.2, label=name)
        mean_v = float(np.mean(data))
        if name != "Cumulative Energy":
            ax.axhline(mean_v, color="black", linestyle="--", linewidth=1.0, alpha=0.7, label=f"Mean: {mean_v:.1f} {unit}")
        else:
            final_v = float(data[-1])
            ax.axhline(final_v, color="darkgreen", linestyle="--", linewidth=1.0, alpha=0.7, label=f"Total: {final_v:.2f} mAh")
        ax.set_ylabel(f"{name} [{unit}]", fontsize=10, fontweight="bold")
        ax.grid(True, alpha=0.3, linestyle=":")
        ax.legend(loc="upper right", fontsize=8.5)

    axes[-1].set_xlabel("Elapsed Flight Time [seconds]", fontsize=11, fontweight="bold")
    plt.tight_layout()
    save_figure(fig, img_dir, "TIME", run_id, short_desc)

def plot_mag_plane_projections(
    mx: np.ndarray,
    my: np.ndarray,
    mz: np.ndarray,
    unit: str,
    short_desc: str,
    run_id: int,
    img_dir: Path,
    is_filtered: bool = False,
    custom_limit: Optional[float] = None
):
    n = len(mx)
    if n == 0:
        return

    # Calculate uniform symmetric bounds across all axes
    max_val = max(float(np.max(np.abs(mx))), float(np.max(np.abs(my))), float(np.max(np.abs(mz))))
    limit = custom_limit if (custom_limit is not None and custom_limit > 0) else (max_val * 1.15 if max_val > 0 else 1.0)

    label_type = "Filtered" if is_filtered else "Raw"
    color_xy = "red" if is_filtered else "gray"
    color_xz = "blue" if is_filtered else "gray"
    color_yz = "green" if is_filtered else "gray"
    alpha_val = 0.7 if is_filtered else 0.6

    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    fig.suptitle(f"Magnetometer {label_type} 3-Plane Projections (Middle Flight Data)", fontsize=14, fontweight="bold")

    # 1. XY Plane
    axes[0].axhline(0, color="black", linestyle="--", linewidth=1.0)
    axes[0].axvline(0, color="black", linestyle="--", linewidth=1.0)
    axes[0].scatter(mx, my, c=color_xy, s=10, alpha=alpha_val, label=label_type)
    axes[0].set_xlabel(f"X axis [{unit}]", fontsize=10)
    axes[0].set_ylabel(f"Y axis [{unit}]", fontsize=10)
    axes[0].set_title(f"Magnetometer {label_type} XY Plane", fontsize=11, fontweight="bold")
    axes[0].set_xlim(-limit, limit)
    axes[0].set_ylim(-limit, limit)
    axes[0].set_aspect('equal', adjustable='box')
    axes[0].grid(True, alpha=0.3, linestyle=":")
    axes[0].legend(loc="upper right", fontsize=8.5)

    # 2. XZ Plane
    axes[1].axhline(0, color="black", linestyle="--", linewidth=1.0)
    axes[1].axvline(0, color="black", linestyle="--", linewidth=1.0)
    axes[1].scatter(mx, mz, c=color_xz, s=10, alpha=alpha_val, label=label_type)
    axes[1].set_xlabel(f"X axis [{unit}]", fontsize=10)
    axes[1].set_ylabel(f"Z axis [{unit}]", fontsize=10)
    axes[1].set_title(f"Magnetometer {label_type} XZ Plane", fontsize=11, fontweight="bold")
    axes[1].set_xlim(-limit, limit)
    axes[1].set_ylim(-limit, limit)
    axes[1].set_aspect('equal', adjustable='box')
    axes[1].grid(True, alpha=0.3, linestyle=":")
    axes[1].legend(loc="upper right", fontsize=8.5)

    # 3. YZ Plane
    axes[2].axhline(0, color="black", linestyle="--", linewidth=1.0)
    axes[2].axvline(0, color="black", linestyle="--", linewidth=1.0)
    axes[2].scatter(my, mz, c=color_yz, s=10, alpha=alpha_val, label=label_type)
    axes[2].set_xlabel(f"Y axis [{unit}]", fontsize=10)
    axes[2].set_ylabel(f"Z axis [{unit}]", fontsize=10)
    axes[2].set_title(f"Magnetometer {label_type} YZ Plane", fontsize=11, fontweight="bold")
    axes[2].set_xlim(-limit, limit)
    axes[2].set_ylim(-limit, limit)
    axes[2].set_aspect('equal', adjustable='box')
    axes[2].grid(True, alpha=0.3, linestyle=":")
    axes[2].legend(loc="upper right", fontsize=8.5)

    plt.tight_layout()
    save_figure(fig, img_dir, "PROJ", run_id, short_desc)

def plot_mag_calib_rmse(
    rmse_history: List[float],
    short_desc: str,
    run_id: int,
    img_dir: Path
):
    epochs = len(rmse_history)
    if epochs == 0:
        return

    epoch_x = np.arange(1, epochs + 1)
    fig, ax = plt.subplots(figsize=(10, 5))
    fig.suptitle("Magnetometer Ellipsoid Calibration Convergence (Epoch vs. RMSE)", fontsize=14, fontweight="bold")

    ax.plot(epoch_x, rmse_history, color="#1f77b4", linewidth=1.8, label="Residual RMSE")
    ax.scatter([1, epochs], [rmse_history[0], rmse_history[-1]], color="crimson", zorder=5)

    ax.annotate(f"Initial: {rmse_history[0]:.4f}", xy=(1, rmse_history[0]), xytext=(max(2, int(epochs*0.08)), rmse_history[0]),
                arrowprops=dict(facecolor='black', shrink=0.05, width=1, headwidth=4), fontsize=9)
    ax.annotate(f"Final: {rmse_history[-1]:.4f}", xy=(epochs, rmse_history[-1]), xytext=(max(1, int(epochs*0.72)), rmse_history[-1] + (rmse_history[0]-rmse_history[-1])*0.15),
                arrowprops=dict(facecolor='black', shrink=0.05, width=1, headwidth=4), fontsize=9)

    ax.set_xlabel("Optimization Epoch", fontsize=11, fontweight="bold")
    ax.set_ylabel("Root Mean Squared Error (RMSE)", fontsize=11, fontweight="bold")
    ax.grid(True, alpha=0.3, linestyle=":")
    ax.legend(loc="upper right", fontsize=9.5)

    plt.tight_layout()
    save_figure(fig, img_dir, "RMSE", run_id, short_desc)

def plot_mag_calib_projections(
    raw_mx: np.ndarray,
    raw_my: np.ndarray,
    raw_mz: np.ndarray,
    calib_mx: np.ndarray,
    calib_my: np.ndarray,
    calib_mz: np.ndarray,
    unit: str,
    short_desc: str,
    run_id: int,
    img_dir: Path,
    b0_radius: float
):
    n = len(raw_mx)
    if n == 0:
        return

    # Symmetrical limits to display both raw and calibrated on the same scale
    max_raw = max(float(np.max(np.abs(raw_mx))), float(np.max(np.abs(raw_my))), float(np.max(np.abs(raw_mz))))
    max_calib = max(float(np.max(np.abs(calib_mx))), float(np.max(np.abs(calib_my))), float(np.max(np.abs(calib_mz))))
    max_val = max(max_raw, max_calib, b0_radius)
    limit = max_val * 1.15 if max_val > 0 else 1.0

    # Parametric circle for ideal calibrated sphere reference
    theta = np.linspace(0, 2 * np.pi, 200)
    circ_x = b0_radius * np.cos(theta)
    circ_y = b0_radius * np.sin(theta)

    fig, axes = plt.subplots(1, 3, figsize=(16, 5))
    fig.suptitle("Magnetometer Calibrated vs. Uncalibrated 3-Plane Projections", fontsize=14, fontweight="bold")

    # 1. XY Plane
    axes[0].axhline(0, color="black", linestyle="--", linewidth=0.8)
    axes[0].axvline(0, color="black", linestyle="--", linewidth=0.8)
    axes[0].plot(circ_x, circ_y, color="red", linestyle=":", linewidth=1.5, alpha=0.8, label=f"Target Sphere (R={b0_radius:.2f})")
    axes[0].scatter(raw_mx, raw_my, c="gray", s=10, alpha=0.45, label="Raw (Uncalibrated)")
    axes[0].scatter(calib_mx, calib_my, c="mediumblue", s=10, alpha=0.6, label="Calibrated")
    axes[0].set_xlabel(f"X axis [{unit}]", fontsize=10)
    axes[0].set_ylabel(f"Y axis [{unit}]", fontsize=10)
    axes[0].set_title("Magnetometer XY Plane", fontsize=11, fontweight="bold")
    axes[0].set_xlim(-limit, limit)
    axes[0].set_ylim(-limit, limit)
    axes[0].set_aspect('equal', adjustable='box')
    axes[0].grid(True, alpha=0.3, linestyle=":")
    axes[0].legend(loc="upper right", fontsize=8.0)

    # 2. XZ Plane
    axes[1].axhline(0, color="black", linestyle="--", linewidth=0.8)
    axes[1].axvline(0, color="black", linestyle="--", linewidth=0.8)
    axes[1].plot(circ_x, circ_y, color="red", linestyle=":", linewidth=1.5, alpha=0.8, label=f"Target Sphere (R={b0_radius:.2f})")
    axes[1].scatter(raw_mx, raw_mz, c="gray", s=10, alpha=0.45, label="Raw (Uncalibrated)")
    axes[1].scatter(calib_mx, calib_mz, c="darkcyan", s=10, alpha=0.6, label="Calibrated")
    axes[1].set_xlabel(f"X axis [{unit}]", fontsize=10)
    axes[1].set_ylabel(f"Z axis [{unit}]", fontsize=10)
    axes[1].set_title("Magnetometer XZ Plane", fontsize=11, fontweight="bold")
    axes[1].set_xlim(-limit, limit)
    axes[1].set_ylim(-limit, limit)
    axes[1].set_aspect('equal', adjustable='box')
    axes[1].grid(True, alpha=0.3, linestyle=":")
    axes[1].legend(loc="upper right", fontsize=8.0)

    # 3. YZ Plane
    axes[2].axhline(0, color="black", linestyle="--", linewidth=0.8)
    axes[2].axvline(0, color="black", linestyle="--", linewidth=0.8)
    axes[2].plot(circ_x, circ_y, color="red", linestyle=":", linewidth=1.5, alpha=0.8, label=f"Target Sphere (R={b0_radius:.2f})")
    axes[2].scatter(raw_my, raw_mz, c="gray", s=10, alpha=0.45, label="Raw (Uncalibrated)")
    axes[2].scatter(calib_my, calib_mz, c="forestgreen", s=10, alpha=0.6, label="Calibrated")
    axes[2].set_xlabel(f"Y axis [{unit}]", fontsize=10)
    axes[2].set_ylabel(f"Z axis [{unit}]", fontsize=10)
    axes[2].set_title("Magnetometer YZ Plane", fontsize=11, fontweight="bold")
    axes[2].set_xlim(-limit, limit)
    axes[2].set_ylim(-limit, limit)
    axes[2].set_aspect('equal', adjustable='box')
    axes[2].grid(True, alpha=0.3, linestyle=":")
    axes[2].legend(loc="upper right", fontsize=8.0)

    plt.tight_layout()
    save_figure(fig, img_dir, "PROJ", run_id, short_desc)

def process_mag_calibration(
    mx: np.ndarray,
    my: np.ndarray,
    mz: np.ndarray,
    unit: str,
    run_id: int,
    img_dir: Path,
    max_epochs: int = 500,
    tol: float = 1e-6
):
    print(f"\n>> Running Magnetometer Rotated Ellipsoid Calibration (Adam Optimizer, max {max_epochs} epochs, tol={tol:.1e})...")
    calib_mx, calib_my, calib_mz, info = fit_rotated_ellipsoid(mx, my, mz, max_epochs=max_epochs, lr=0.02, tol=tol)

    v0 = info['v0']
    W = info['W']
    p9 = info['params_9']
    B0 = info['B0']
    init_rmse = info['initial_rmse']
    final_rmse = info['final_rmse']
    imp_pct = ((init_rmse - final_rmse) / init_rmse * 100.0) if init_rmse > 0 else 0.0
    status_str = f"Converged early in {info['epochs_run']} epochs" if info['converged'] else f"Completed max {info['epochs_run']} epochs"

    print("==========================================================================================")
    print("  MAGNETOMETER CALIBRATION RESULTS (Symmetric Soft-Iron + Hard-Iron Fit)")
    print("------------------------------------------------------------------------------------------")
    print(f"  Status:                 {status_str}")
    print(f"  Initial Residual RMSE:  {init_rmse:.6f}")
    print(f"  Final Residual RMSE:    {final_rmse:.6f}  (Improvement: {imp_pct:.2f}%)")
    print(f"  Target Sphere Radius:   {B0:.4f} {unit}")
    print("------------------------------------------------------------------------------------------")
    print("  THE 9 CALIBRATED MODEL PARAMETERS:")
    print("  1. Hard-Iron Offset Vector (v0):")
    print(f"       v0_x = {p9['v0_x']:+.6f} {unit}")
    print(f"       v0_y = {p9['v0_y']:+.6f} {unit}")
    print(f"       v0_z = {p9['v0_z']:+.6f} {unit}")
    print("  2. Soft-Iron Symmetric Matrix Parameters (W = W^T):")
    print(f"       W_11 = {p9['W_11']:+.6f}        W_12 = {p9['W_12']:+.6f}")
    print(f"       W_22 = {p9['W_22']:+.6f}        W_13 = {p9['W_13']:+.6f}")
    print(f"       W_33 = {p9['W_33']:+.6f}        W_23 = {p9['W_23']:+.6f}")
    print("  3. Full Soft-Iron Symmetric Matrix (W):")
    print(f"       [ {W[0,0]:+.6f}, {W[0,1]:+.6f}, {W[0,2]:+.6f} ]")
    print(f"       [ {W[1,0]:+.6f}, {W[1,1]:+.6f}, {W[1,2]:+.6f} ]")
    print(f"       [ {W[2,0]:+.6f}, {W[2,1]:+.6f}, {W[2,2]:+.6f} ]")
    print("==========================================================================================\n")

    # Plot 1: Epoch vs RMSE
    plot_mag_calib_rmse(info['rmse_history'], "Mag_calib_rmse", run_id, img_dir)

    # Plot 2: Uncalibrated vs Calibrated 3-Plane Projections
    plot_mag_calib_projections(mx, my, mz, calib_mx, calib_my, calib_mz, unit, "Mag_calib_proj", run_id, img_dir, B0)

# =============================================================================
# 5. STREAM PROCESSING DISPATCHER
# =============================================================================
def process_accelerometer(
    log_data: FlightLogData,
    do_raw_hist: bool,
    do_raw_fft: bool,
    do_raw_time: bool,
    do_filt_hist: bool,
    do_filt_fft: bool,
    do_filt_time: bool,
    run_id: int,
    img_dir: Path,
    trim_start: float,
    trim_end: float
):
    has_raw = log_data.imu_raw is not None and len(log_data.imu_raw) > 0
    has_filt = log_data.imu_filt is not None and len(log_data.imu_filt) > 0

    if not has_raw and not has_filt:
        print("[!] No Accelerometer data found in flight log.")
        return

    # 1. RAW Accelerometer Processing
    if has_raw and (do_raw_hist or do_raw_fft or do_raw_time):
        raw_data, s_idx, e_idx = trim_data_stream(log_data.imu_raw, trim_start, trim_end)
        print(f"\n>> Processing Accelerometer RAW: {len(raw_data)} samples analyzed (trimmed {s_idx} to {e_idx} of {len(log_data.imu_raw)})")

        ts = raw_data['ts_us']
        ax = raw_data['ax']
        ay = raw_data['ay']
        az = raw_data['az']

        stats = [
            calculate_stream_stats("Accel X (Raw)", ts, ax, "g"),
            calculate_stream_stats("Accel Y (Raw)", ts, ay, "g"),
            calculate_stream_stats("Accel Z (Raw)", ts, az, "g")
        ]
        print_stats_table(stats, f"Accelerometer RAW (Session LOG_{log_data.session_id:04d})")

        filt_data = None
        if has_filt:
            filt_data, _, _ = trim_data_stream(log_data.imu_filt, trim_start, trim_end)

        if do_raw_hist:
            plot_histogram_3axis(ts, ax, ay, az, "Accelerometer RAW", "g", "Accel_raw", run_id, img_dir, color="#4C72B0")

        if do_raw_fft:
            plot_fft_3axis(ts, ax, ay, az, "Accelerometer RAW", "g", "Accel_raw", run_id, img_dir, color="midnightblue")

        if do_raw_time:
            if filt_data is not None:
                plot_time_series_3axis(
                    ts, ax, ay, az, "Accelerometer RAW", "g", "Accel_raw", run_id, img_dir,
                    filt_ts_us=filt_data['ts_us'], filt_x=filt_data['ax'], filt_y=filt_data['ay'], filt_z=filt_data['az']
                )
            else:
                plot_time_series_3axis(ts, ax, ay, az, "Accelerometer RAW", "g", "Accel_raw", run_id, img_dir)

    elif not has_raw and (do_raw_hist or do_raw_fft or do_raw_time):
        print("  [!] No Raw Accelerometer data found in flight log.")

    # 2. FILTERED Accelerometer Processing
    if has_filt and (do_filt_hist or do_filt_fft or do_filt_time):
        filt_data, fs_idx, fe_idx = trim_data_stream(log_data.imu_filt, trim_start, trim_end)
        print(f"\n>> Processing Accelerometer FILTERED: {len(filt_data)} samples analyzed (trimmed {fs_idx} to {fe_idx} of {len(log_data.imu_filt)})")

        filt_ts = filt_data['ts_us']
        filt_ax = filt_data['ax']
        filt_ay = filt_data['ay']
        filt_az = filt_data['az']

        stats_filt = [
            calculate_stream_stats("Accel X (Filt)", filt_ts, filt_ax, "g"),
            calculate_stream_stats("Accel Y (Filt)", filt_ts, filt_ay, "g"),
            calculate_stream_stats("Accel Z (Filt)", filt_ts, filt_az, "g")
        ]
        print_stats_table(stats_filt, f"Accelerometer FILTERED (Session LOG_{log_data.session_id:04d})")

        if do_filt_hist:
            plot_histogram_3axis(filt_ts, filt_ax, filt_ay, filt_az, "Accelerometer FILTERED", "g", "Accel_filt", run_id, img_dir, color="#5B92E5")

        if do_filt_fft:
            plot_fft_3axis(filt_ts, filt_ax, filt_ay, filt_az, "Accelerometer FILTERED", "g", "Accel_filt", run_id, img_dir, color="#1D4E89")

        if do_filt_time:
            plot_time_series_3axis(filt_ts, filt_ax, filt_ay, filt_az, "Accelerometer FILTERED", "g", "Accel_filt", run_id, img_dir)

    elif not has_filt and (do_filt_hist or do_filt_fft or do_filt_time):
        print("  [!] No Filtered Accelerometer data found in flight log.")

def process_gyroscope(
    log_data: FlightLogData,
    do_raw_hist: bool,
    do_raw_fft: bool,
    do_raw_time: bool,
    do_filt_hist: bool,
    do_filt_fft: bool,
    do_filt_time: bool,
    run_id: int,
    img_dir: Path,
    trim_start: float,
    trim_end: float
):
    has_raw = log_data.imu_raw is not None and len(log_data.imu_raw) > 0
    has_filt = log_data.imu_filt is not None and len(log_data.imu_filt) > 0

    if not has_raw and not has_filt:
        print("[!] No Gyroscope data found in flight log.")
        return

    # 1. RAW Gyroscope Processing
    if has_raw and (do_raw_hist or do_raw_fft or do_raw_time):
        raw_data, s_idx, e_idx = trim_data_stream(log_data.imu_raw, trim_start, trim_end)
        print(f"\n>> Processing Gyroscope RAW: {len(raw_data)} samples analyzed (trimmed {s_idx} to {e_idx} of {len(log_data.imu_raw)})")

        ts = raw_data['ts_us']
        # Firmware MPU6050 driver already converts raw ADC values to deg/s via inv_gyro_sens (1/131.0 LSB/(deg/s)).
        # Data is already in deg/s. Do not apply np.degrees() to avoid false 57.3x multiplication!
        gx = raw_data['gx']
        gy = raw_data['gy']
        gz = raw_data['gz']

        stats = [
            calculate_stream_stats("Gyro X (Raw)", ts, gx, "deg/s"),
            calculate_stream_stats("Gyro Y (Raw)", ts, gy, "deg/s"),
            calculate_stream_stats("Gyro Z (Raw)", ts, gz, "deg/s")
        ]
        print_stats_table(stats, f"Gyroscope RAW (Session LOG_{log_data.session_id:04d})")

        filt_data = None
        if has_filt:
            filt_data, _, _ = trim_data_stream(log_data.imu_filt, trim_start, trim_end)

        if do_raw_hist:
            plot_histogram_3axis(ts, gx, gy, gz, "Gyroscope RAW", "deg/s", "Gyro_raw", run_id, img_dir, color="#C44E52")

        if do_raw_fft:
            plot_fft_3axis(ts, gx, gy, gz, "Gyroscope RAW", "deg/s", "Gyro_raw", run_id, img_dir, color="#8172B2")

        if do_raw_time:
            if filt_data is not None:
                plot_time_series_3axis(
                    ts, gx, gy, gz, "Gyroscope RAW", "deg/s", "Gyro_raw", run_id, img_dir,
                    filt_ts_us=filt_data['ts_us'], filt_x=filt_data['gx'], filt_y=filt_data['gy'], filt_z=filt_data['gz']
                )
            else:
                plot_time_series_3axis(ts, gx, gy, gz, "Gyroscope RAW", "deg/s", "Gyro_raw", run_id, img_dir)

    elif not has_raw and (do_raw_hist or do_raw_fft or do_raw_time):
        print("  [!] No Raw Gyroscope data found in flight log.")

    # 2. FILTERED Gyroscope Processing
    if has_filt and (do_filt_hist or do_filt_fft or do_filt_time):
        filt_data, fs_idx, fe_idx = trim_data_stream(log_data.imu_filt, trim_start, trim_end)
        print(f"\n>> Processing Gyroscope FILTERED: {len(filt_data)} samples analyzed (trimmed {fs_idx} to {fe_idx} of {len(log_data.imu_filt)})")

        filt_ts = filt_data['ts_us']
        filt_gx = filt_data['gx']
        filt_gy = filt_data['gy']
        filt_gz = filt_data['gz']

        stats_filt = [
            calculate_stream_stats("Gyro X (Filt)", filt_ts, filt_gx, "deg/s"),
            calculate_stream_stats("Gyro Y (Filt)", filt_ts, filt_gy, "deg/s"),
            calculate_stream_stats("Gyro Z (Filt)", filt_ts, filt_gz, "deg/s")
        ]
        print_stats_table(stats_filt, f"Gyroscope FILTERED (Session LOG_{log_data.session_id:04d})")

        if do_filt_hist:
            plot_histogram_3axis(filt_ts, filt_gx, filt_gy, filt_gz, "Gyroscope FILTERED", "deg/s", "Gyro_filt", run_id, img_dir, color="#E26D5C")

        if do_filt_fft:
            plot_fft_3axis(filt_ts, filt_gx, filt_gy, filt_gz, "Gyroscope FILTERED", "deg/s", "Gyro_filt", run_id, img_dir, color="#5E4FA2")

        if do_filt_time:
            plot_time_series_3axis(filt_ts, filt_gx, filt_gy, filt_gz, "Gyroscope FILTERED", "deg/s", "Gyro_filt", run_id, img_dir)

    elif not has_filt and (do_filt_hist or do_filt_fft or do_filt_time):
        print("  [!] No Filtered Gyroscope data found in flight log.")

def process_magnetometer(
    log_data: FlightLogData,
    do_raw_hist: bool,
    do_raw_fft: bool,
    do_raw_time: bool,
    do_raw_proj: bool,
    do_raw_calib: bool,
    do_filt_hist: bool,
    do_filt_fft: bool,
    do_filt_time: bool,
    do_filt_proj: bool,
    run_id: int,
    img_dir: Path,
    trim_start: float,
    trim_end: float
):
    has_raw = log_data.mag_raw is not None and len(log_data.mag_raw) > 0
    has_filt = log_data.mag_filt is not None and len(log_data.mag_filt) > 0

    if not has_raw and not has_filt:
        print("[!] No Magnetometer data found in flight log.")
        return

    # 1. RAW Magnetometer Processing
    if has_raw and (do_raw_hist or do_raw_fft or do_raw_time or do_raw_proj or do_raw_calib):
        raw_data, s_idx, e_idx = trim_data_stream(log_data.mag_raw, trim_start, trim_end)
        print(f"\n>> Processing Magnetometer RAW: {len(raw_data)} samples analyzed (trimmed {s_idx} to {e_idx} of {len(log_data.mag_raw)})")

        ts = raw_data['ts_us']
        mx = raw_data['mx']
        my = raw_data['my']
        mz = raw_data['mz']

        stats = [
            calculate_stream_stats("Mag X (Raw)", ts, mx, "Gauss"),
            calculate_stream_stats("Mag Y (Raw)", ts, my, "Gauss"),
            calculate_stream_stats("Mag Z (Raw)", ts, mz, "Gauss")
        ]
        print_stats_table(stats, f"Magnetometer RAW (Session LOG_{log_data.session_id:04d})")

        filt_data = None
        if has_filt:
            filt_data, _, _ = trim_data_stream(log_data.mag_filt, trim_start, trim_end)

        if do_raw_hist:
            plot_histogram_3axis(ts, mx, my, mz, "Magnetometer RAW", "Gauss", "Mag_raw", run_id, img_dir, color="#55A868")

        if do_raw_fft:
            plot_fft_3axis(ts, mx, my, mz, "Magnetometer RAW", "Gauss", "Mag_raw", run_id, img_dir, color="#2B7A4B")

        if do_raw_time:
            if filt_data is not None:
                plot_time_series_3axis(
                    ts, mx, my, mz, "Magnetometer RAW", "Gauss", "Mag_raw", run_id, img_dir,
                    filt_ts_us=filt_data['ts_us'], filt_x=filt_data['mx'], filt_y=filt_data['my'], filt_z=filt_data['mz']
                )
            else:
                plot_time_series_3axis(ts, mx, my, mz, "Magnetometer RAW", "Gauss", "Mag_raw", run_id, img_dir)

        if do_raw_proj:
            plot_mag_plane_projections(mx, my, mz, "Gauss", "Mag_raw", run_id, img_dir, is_filtered=False)

        if do_raw_calib:
            process_mag_calibration(mx, my, mz, "Gauss", run_id, img_dir)

    elif not has_raw and (do_raw_hist or do_raw_fft or do_raw_time or do_raw_proj or do_raw_calib):
        print("  [!] No Raw Magnetometer data found in flight log.")

    # 2. FILTERED Magnetometer Processing
    if has_filt and (do_filt_hist or do_filt_fft or do_filt_time or do_filt_proj):
        filt_data, fs_idx, fe_idx = trim_data_stream(log_data.mag_filt, trim_start, trim_end)
        print(f"\n>> Processing Magnetometer FILTERED: {len(filt_data)} samples analyzed (trimmed {fs_idx} to {fe_idx} of {len(log_data.mag_filt)})")

        filt_ts = filt_data['ts_us']
        filt_mx = filt_data['mx']
        filt_my = filt_data['my']
        filt_mz = filt_data['mz']

        stats_filt = [
            calculate_stream_stats("Mag X (Filt)", filt_ts, filt_mx, "Gauss"),
            calculate_stream_stats("Mag Y (Filt)", filt_ts, filt_my, "Gauss"),
            calculate_stream_stats("Mag Z (Filt)", filt_ts, filt_mz, "Gauss")
        ]
        print_stats_table(stats_filt, f"Magnetometer FILTERED (Session LOG_{log_data.session_id:04d})")

        if do_filt_hist:
            plot_histogram_3axis(filt_ts, filt_mx, filt_my, filt_mz, "Magnetometer FILTERED", "Gauss", "Mag_filt", run_id, img_dir, color="#2CA25F")

        if do_filt_fft:
            plot_fft_3axis(filt_ts, filt_mx, filt_my, filt_mz, "Magnetometer FILTERED", "Gauss", "Mag_filt", run_id, img_dir, color="#006D2C")

        if do_filt_time:
            plot_time_series_3axis(filt_ts, filt_mx, filt_my, filt_mz, "Magnetometer FILTERED", "Gauss", "Mag_filt", run_id, img_dir)

        if do_filt_proj:
            plot_mag_plane_projections(filt_mx, filt_my, filt_mz, "Gauss", "Mag_filt", run_id, img_dir, is_filtered=True)

    elif not has_filt and (do_filt_hist or do_filt_fft or do_filt_time or do_filt_proj):
        print("  [!] No Filtered Magnetometer data found in flight log.")

def process_barometer(
    log_data: FlightLogData,
    do_raw_hist: bool,
    do_raw_fft: bool,
    do_raw_time: bool,
    do_filt_hist: bool,
    do_filt_fft: bool,
    do_filt_time: bool,
    run_id: int,
    img_dir: Path,
    trim_start: float,
    trim_end: float
):
    has_raw = log_data.baro_raw is not None and len(log_data.baro_raw) > 0
    has_filt = log_data.baro_filt is not None and len(log_data.baro_filt) > 0

    if not has_raw and not has_filt:
        print("[!] No Barometer data found in flight log.")
        return

    # 1. RAW Barometer Processing
    if has_raw and (do_raw_hist or do_raw_fft or do_raw_time):
        if len(log_data.baro_raw) < 16 and (do_raw_hist or do_raw_fft):
            print(f"  [!] Barometer RAW sample count ({len(log_data.baro_raw)}) is too small for statistical distribution/FFT analysis.")
        else:
            raw_data, s_idx, e_idx = trim_data_stream(log_data.baro_raw, trim_start, trim_end)
            print(f"\n>> Processing Barometer RAW: {len(raw_data)} samples analyzed (trimmed {s_idx} to {e_idx} of {len(log_data.baro_raw)})")

            ts = raw_data['ts_us']
            press = raw_data['press']
            alt = 44330.0 * (1.0 - (press / P0_SEA_LEVEL_PA) ** (1.0 / 5.255))

            stats = [
                calculate_stream_stats("Pressure (Raw)", ts, press, "Pa"),
                calculate_stream_stats("Est Alt (Raw)", ts, alt, "m")
            ]
            print_stats_table(stats, f"Barometer RAW (Session LOG_{log_data.session_id:04d})")

            filt_data = None
            if has_filt:
                filt_data, _, _ = trim_data_stream(log_data.baro_filt, trim_start, trim_end)

            if do_raw_hist:
                plot_baro_histogram(ts, press, "Baro_raw", run_id, img_dir, sensor_name="Barometer RAW")

            if do_raw_fft:
                plot_baro_fft(ts, press, "Baro_raw", run_id, img_dir, sensor_name="Barometer RAW")

            if do_raw_time:
                if filt_data is not None:
                    plot_baro_time_series(ts, press, "Baro_raw", run_id, img_dir, filt_ts_us=filt_data['ts_us'], filt_pressure_pa=filt_data['press'], sensor_name="Barometer RAW")
                else:
                    plot_baro_time_series(ts, press, "Baro_raw", run_id, img_dir, sensor_name="Barometer RAW")

    elif not has_raw and (do_raw_hist or do_raw_fft or do_raw_time):
        print("  [!] No Raw Barometer data found in flight log.")

    # 2. FILTERED Barometer Processing
    if has_filt and (do_filt_hist or do_filt_fft or do_filt_time):
        if len(log_data.baro_filt) < 16 and (do_filt_hist or do_filt_fft):
            print(f"  [!] Barometer FILTERED sample count ({len(log_data.baro_filt)}) is too small for statistical distribution/FFT analysis.")
        else:
            filt_data, fs_idx, fe_idx = trim_data_stream(log_data.baro_filt, trim_start, trim_end)
            print(f"\n>> Processing Barometer FILTERED: {len(filt_data)} samples analyzed (trimmed {fs_idx} to {fe_idx} of {len(log_data.baro_filt)})")

            filt_ts = filt_data['ts_us']
            filt_press = filt_data['press']
            filt_alt = 44330.0 * (1.0 - (filt_press / P0_SEA_LEVEL_PA) ** (1.0 / 5.255))

            stats_filt = [
                calculate_stream_stats("Pressure (Filt)", filt_ts, filt_press, "Pa"),
                calculate_stream_stats("Est Alt (Filt)", filt_ts, filt_alt, "m")
            ]
            print_stats_table(stats_filt, f"Barometer FILTERED (Session LOG_{log_data.session_id:04d})")

            if do_filt_hist:
                plot_baro_histogram(filt_ts, filt_press, "Baro_filt", run_id, img_dir, sensor_name="Barometer FILTERED")

            if do_filt_fft:
                plot_baro_fft(filt_ts, filt_press, "Baro_filt", run_id, img_dir, sensor_name="Barometer FILTERED")

            if do_filt_time:
                plot_baro_time_series(filt_ts, filt_press, "Baro_filt", run_id, img_dir, sensor_name="Barometer FILTERED")

    elif not has_filt and (do_filt_hist or do_filt_fft or do_filt_time):
        print("  [!] No Filtered Barometer data found in flight log.")

def process_attitude(
    log_data: FlightLogData,
    do_time: bool,
    run_id: int,
    img_dir: Path,
    trim_start: float,
    trim_end: float
):
    if log_data.att_euler is None or len(log_data.att_euler) == 0:
        print("[!] No Attitude Euler data found in flight log.")
        return

    raw_data, s_idx, e_idx = trim_data_stream(log_data.att_euler, trim_start, trim_end)
    print(f"\n>> Processing Attitude: {len(raw_data)} samples analyzed (trimmed {s_idx} to {e_idx} of {len(log_data.att_euler)})")

    ts = raw_data['ts_us']
    roll = raw_data['roll']
    pitch = raw_data['pitch']
    yaw = raw_data['yaw']

    stats = [
        calculate_stream_stats("Roll", ts, roll, "deg"),
        calculate_stream_stats("Pitch", ts, pitch, "deg"),
        calculate_stream_stats("Yaw", ts, yaw, "deg")
    ]
    print_stats_table(stats, f"Attitude Euler (Session LOG_{log_data.session_id:04d})")

    if do_time:
        plot_attitude_time_series(ts, roll, pitch, yaw, "Attitude_euler", run_id, img_dir)

def process_power(
    log_data: FlightLogData,
    do_hist: bool,
    do_time: bool,
    run_id: int,
    img_dir: Path,
    trim_start: float,
    trim_end: float
):
    if log_data.power is None or len(log_data.power) == 0:
        print("[!] No Power (INA226) data found in flight log.")
        return

    raw_data, s_idx, e_idx = trim_data_stream(log_data.power, trim_start, trim_end)
    print(f"\n>> Processing Power Module: {len(raw_data)} samples analyzed (trimmed {s_idx} to {e_idx} of {len(log_data.power)})")

    ts = raw_data['ts_us']
    v_mv = raw_data['voltage_mv']
    i_ma = raw_data['current_ma']
    p_mw = raw_data['power_mw']
    e_mah = raw_data['energy_mah']

    stats = [
        calculate_stream_stats("Voltage", ts, v_mv, "mV"),
        calculate_stream_stats("Current", ts, i_ma, "mA"),
        calculate_stream_stats("Power", ts, p_mw, "mW"),
        calculate_stream_stats("Energy", ts, e_mah, "mAh")
    ]
    print_stats_table(stats, f"Power Monitor INA226 (Session LOG_{log_data.session_id:04d})")

    # High-level electrical metrics
    v_initial = float(v_mv[0])
    v_min = float(np.min(v_mv))
    v_sag = v_initial - v_min
    i_peak = float(np.max(i_ma))
    i_avg = float(np.mean(i_ma))
    p_peak = float(np.max(p_mw))
    p_avg = float(np.mean(p_mw))
    total_mah = float(e_mah[-1] - e_mah[0])

    print("------------------------------------------------------------------------------------------")
    print("  POWER METRICS SUMMARY:")
    print(f"    Initial Pack Voltage:    {v_initial:.1f} mV ({v_initial / 1000.0:.2f} V)")
    print(f"    Minimum Loaded Voltage:  {v_min:.1f} mV ({v_min / 1000.0:.2f} V)")
    print(f"    Maximum Voltage Sag:     {v_sag:.1f} mV ({v_sag / 1000.0:.2f} V)")
    print(f"    Average Current Draw:    {i_avg:.1f} mA ({i_avg / 1000.0:.2f} A)")
    print(f"    Peak Current Draw:       {i_peak:.1f} mA ({i_peak / 1000.0:.2f} A)")
    print(f"    Average Power Usage:     {p_avg:.1f} mW ({p_avg / 1000.0:.2f} W)")
    print(f"    Peak Power Usage:        {p_peak:.1f} mW ({p_peak / 1000.0:.2f} W)")
    print(f"    Energy Consumed (Window):{total_mah:.2f} mAh")
    print("------------------------------------------------------------------------------------------\n")

    if do_hist:
        plot_power_histogram(v_mv, i_ma, p_mw, "Power_ina226", run_id, img_dir)

    if do_time:
        plot_power_time_series(ts, v_mv, i_ma, p_mw, e_mah, "Power_ina226", run_id, img_dir)

# =============================================================================
# 6. SCAN REPORT & MENU
# =============================================================================
def print_scan_report(log_data: FlightLogData):
    print("\n" + "=" * 80)
    print(f"  FLIGHT LOG SCAN REPORT: {log_data.filepath.name}")
    print(f"  Target Board:     {log_data.board_name}")
    print(f"  Session ID:       LOG_{log_data.session_id:04d}")
    print(f"  Boot Timestamp:   {log_data.boot_ts_us} us ({log_data.boot_ts_us / 1e6:.3f} s)")
    print(f"  Limit Policy:     {log_data.limit_policy}")
    print(f"  Max File Size:    {log_data.max_file_size / (1024*1024):.1f} MB")
    print("=" * 80)
    print("  Tracked Data Streams in Log:")

    def describe_stream(name: str, arr: Optional[np.ndarray], nominal_hz: int):
        if arr is None or len(arr) == 0:
            print(f"    [ ] {name:<32}: Not logged")
        else:
            dur_s = (float(arr['ts_us'][-1]) - float(arr['ts_us'][0])) / 1e6 if len(arr) > 1 else 0.0
            print(f"    [OK] {name:<32}: {len(arr):>6,d} samples (~{dur_s:5.2f} s @ ~{nominal_hz} Hz)")

    describe_stream("Accelerometer & Gyroscope RAW", log_data.imu_raw, 500)
    describe_stream("Accelerometer & Gyroscope FILT", log_data.imu_filt, 500)
    describe_stream("Magnetometer RAW", log_data.mag_raw, 100)
    describe_stream("Magnetometer FILT", log_data.mag_filt, 100)
    describe_stream("Barometer RAW", log_data.baro_raw, 50)
    describe_stream("Barometer FILT", log_data.baro_filt, 50)
    describe_stream("Attitude Euler Angles", log_data.att_euler, 50)
    describe_stream("Attitude Quaternions", log_data.att_quat, 50)
    describe_stream("Power Monitor (INA226)", log_data.power, 50)
    print("=" * 80 + "\n")

def display_menu(run_id: int):
    print("------------------------------------------------------------")
    print(f"  ANALYSIS SELECTION MENU (Assigned Run ID: {run_id:04d})")
    print("------------------------------------------------------------")
    print("  ACCELEROMETER:")
    print("    [1]  Accelerometer RAW      - Histogram & Standard Deviation")
    print("    [2]  Accelerometer RAW      - Fast Fourier Transform (FFT)")
    print("    [3]  Accelerometer RAW      - Time Series (Value vs. Time)")
    print("    [4]  Accelerometer FILTERED - Histogram & Standard Deviation")
    print("    [5]  Accelerometer FILTERED - Fast Fourier Transform (FFT)")
    print("    [6]  Accelerometer FILTERED - Time Series (Value vs. Time)")
    print("")
    print("  GYROSCOPE:")
    print("    [7]  Gyroscope RAW          - Histogram & Standard Deviation")
    print("    [8]  Gyroscope RAW          - Fast Fourier Transform (FFT)")
    print("    [9]  Gyroscope RAW          - Time Series (Value vs. Time)")
    print("   [10]  Gyroscope FILTERED     - Histogram & Standard Deviation")
    print("   [11]  Gyroscope FILTERED     - Fast Fourier Transform (FFT)")
    print("   [12]  Gyroscope FILTERED     - Time Series (Value vs. Time)")
    print("")
    print("  MAGNETOMETER:")
    print("   [13]  Magnetometer RAW       - Histogram & Standard Deviation")
    print("   [14]  Magnetometer RAW       - Fast Fourier Transform (FFT)")
    print("   [15]  Magnetometer RAW       - Time Series (Value vs. Time)")
    print("   [16]  Magnetometer RAW       - 3-Plane Projections")
    print("   [17]  Magnetometer RAW       - Ellipsoid Calibration (Rotated Fit & Epoch RMSE)")
    print("   [18]  Magnetometer FILTERED  - Histogram & Standard Deviation")
    print("   [19]  Magnetometer FILTERED  - Fast Fourier Transform (FFT)")
    print("   [20]  Magnetometer FILTERED  - Time Series (Value vs. Time)")
    print("   [21]  Magnetometer FILTERED  - 3-Plane Projections")
    print("")
    print("  BAROMETER:")
    print("   [22]  Barometer RAW          - Histogram & Standard Deviation")
    print("   [23]  Barometer RAW          - Fast Fourier Transform (FFT)")
    print("   [24]  Barometer RAW          - Time Series (Value vs. Time)")
    print("   [25]  Barometer FILTERED     - Histogram & Standard Deviation")
    print("   [26]  Barometer FILTERED     - Fast Fourier Transform (FFT)")
    print("   [27]  Barometer FILTERED     - Time Series (Value vs. Time)")
    print("")
    print("  ATTITUDE (ORIENTATION):")
    print("   [28]  Attitude Euler         - Time Series (Roll / Pitch / Yaw vs. Time)")
    print("")
    print("  POWER MONITOR (INA226):")
    print("   [29]  Power                  - Histogram & Standard Deviation (Voltage / Current / Power)")
    print("   [30]  Power                  - Time Series (Voltage, Current, Power, Energy vs. Time)")
    print("")
    print("  BATCH / COMBINED:")
    print("   [31]  All Time Series Plots (Raw & Filtered)")
    print("   [32]  All Histograms (Raw & Filtered)")
    print("   [33]  All FFTs (Raw & Filtered)")
    print("   [34]  All RAW Analysis (Histograms, FFTs, Time Series, Projections, Calibration)")
    print("   [35]  All FILTERED Analysis (Histograms, FFTs, Time Series, Projections)")
    print("   [36]  Complete Analysis (Everything: All Raw + All Filtered)")
    print("    [0]  Exit")
    print("------------------------------------------------------------")

def parse_user_choices(raw_input: str) -> List[int]:
    """Parses user input like '1, 2, 4' or '1-4' into integer list."""
    choices = set()
    parts = raw_input.replace(" ", "").split(",")
    for part in parts:
        if not part:
            continue
        if "-" in part:
            bounds = part.split("-")
            if len(bounds) == 2 and bounds[0].isdigit() and bounds[1].isdigit():
                low, high = int(bounds[0]), int(bounds[1])
                for v in range(min(low, high), max(low, high) + 1):
                    choices.add(v)
        elif part.isdigit():
            choices.add(int(part))
    return sorted(list(choices))

# =============================================================================
# 7. MAIN ORCHESTRATION
# =============================================================================
def main():
    parser = argparse.ArgumentParser(
        description="Universal Flight Log Decoder, Trimmer & Spectral Analyzer"
    )
    parser.add_argument("logfile", nargs="?", help="Path to LOG_xxxx.BIN file (default: newest in binary_black_box_data/)")
    parser.add_argument("--select", type=str, help="Non-interactive menu choices (e.g., '1,4,7' or '36')")
    parser.add_argument("--all", action="store_true", help="Non-interactive full analysis shortcut (same as --select 36)")
    parser.add_argument("--raw", action="store_true", help="Non-interactive all RAW analysis shortcut (same as --select 34)")
    parser.add_argument("--filt", action="store_true", help="Non-interactive all FILTERED analysis shortcut (same as --select 35)")
    parser.add_argument("--time", action="store_true", help="Non-interactive all time-series shortcut (same as --select 31)")
    parser.add_argument("--hist", action="store_true", help="Non-interactive all histograms shortcut (same as --select 32)")
    parser.add_argument("--fft", action="store_true", help="Non-interactive all FFTs shortcut (same as --select 33)")
    parser.add_argument("--trim-start", type=float, default=10.0, help="Initial data percentage to cut (default: 10.0%%)")
    parser.add_argument("--trim-end", type=float, default=10.0, help="Final data percentage to cut (default: 10.0%%)")
    args = parser.parse_args()

    # Determine next stateless run ID directly from img/ and log/
    img_dir = get_img_dir()
    log_dir = get_log_dir()
    run_id = determine_next_run_id(img_dir, log_dir)

    # Initialize terminal output mirroring to YYYY-LOG.txt (saved exclusively in log/)
    log_file = log_dir / f"{run_id:04d}-LOG.txt"
    tee = TeeLogger([log_file])
    orig_stdout = sys.stdout
    sys.stdout = tee

    try:
        # Determine file path
        if args.logfile:
            p = Path(args.logfile)
            if not p.is_absolute():
                p = (get_script_dir() / p).resolve()
            target_path = p
        else:
            print("[INFO] Opening File Explorer to select binary flight log (.BIN)...")
            chosen_file = select_file_via_explorer()
            if chosen_file:
                target_path = chosen_file
                print(f"[INFO] Selected file via Explorer: {target_path}")
            else:
                print("[INFO] No file selected in dialog. Falling back to newest available file...")
                target_path = find_default_log_file()
                if target_path is None:
                    print("[ERROR] No .BIN flight log files found in analyzers/ or analyzers/binary_black_box_data/.")
                    sys.exit(1)
                print(f"[INFO] Auto-selected fallback log file: {target_path}")

        print(f"[INFO] Initialized next Run ID: {run_id:04d}")

        # 1. Parse binary log
        log_data = parse_binary_log(target_path)
        if log_data is None:
            sys.exit(1)

        # 2. Print initial scan report
        print_scan_report(log_data)

        # 3. Handle selection (CLI arguments or interactive prompt)
        if args.all:
            selected_options = [36]
        elif args.raw:
            selected_options = [34]
        elif args.filt:
            selected_options = [35]
        elif args.time:
            selected_options = [31]
        elif args.hist:
            selected_options = [32]
        elif args.fft:
            selected_options = [33]
        elif args.select:
            selected_options = parse_user_choices(args.select)
        else:
            display_menu(run_id)
            try:
                user_in = input("Enter option number(s) [e.g. 1, 4, 7 or 1-6 or 36]: ").strip()
                if not user_in:
                    print("[INFO] No option entered. Defaulting to option 36 (Complete Analysis).")
                    selected_options = [36]
                else:
                    selected_options = parse_user_choices(user_in)
            except (KeyboardInterrupt, EOFError):
                print("\n[INFO] Exiting on user request.")
                sys.exit(0)

        if 0 in selected_options:
            print("[INFO] Exiting.")
            sys.exit(0)

        # Resolve batch shortcuts
        all_complete = (36 in selected_options)
        all_raw      = (34 in selected_options) or all_complete
        all_filt     = (35 in selected_options) or all_complete
        all_time     = (31 in selected_options) or all_complete
        all_hist     = (32 in selected_options) or all_complete
        all_fft      = (33 in selected_options) or all_complete

        do_accel_raw_hist  = (1 in selected_options) or all_raw or all_hist
        do_accel_raw_fft   = (2 in selected_options) or all_raw or all_fft
        do_accel_raw_time  = (3 in selected_options) or all_raw or all_time
        do_accel_filt_hist = (4 in selected_options) or all_filt or all_hist
        do_accel_filt_fft  = (5 in selected_options) or all_filt or all_fft
        do_accel_filt_time = (6 in selected_options) or all_filt or all_time

        do_gyro_raw_hist   = (7 in selected_options) or all_raw or all_hist
        do_gyro_raw_fft    = (8 in selected_options) or all_raw or all_fft
        do_gyro_raw_time   = (9 in selected_options) or all_raw or all_time
        do_gyro_filt_hist  = (10 in selected_options) or all_filt or all_hist
        do_gyro_filt_fft   = (11 in selected_options) or all_filt or all_fft
        do_gyro_filt_time  = (12 in selected_options) or all_filt or all_time

        do_mag_raw_hist    = (13 in selected_options) or all_raw or all_hist
        do_mag_raw_fft     = (14 in selected_options) or all_raw or all_fft
        do_mag_raw_time    = (15 in selected_options) or all_raw or all_time
        do_mag_raw_proj    = (16 in selected_options) or all_raw or all_complete
        do_mag_raw_calib   = (17 in selected_options) or all_raw or all_complete

        do_mag_filt_hist   = (18 in selected_options) or all_filt or all_hist
        do_mag_filt_fft    = (19 in selected_options) or all_filt or all_fft
        do_mag_filt_time   = (20 in selected_options) or all_filt or all_time
        do_mag_filt_proj   = (21 in selected_options) or all_filt or all_complete

        do_baro_raw_hist   = (22 in selected_options) or all_raw or all_hist
        do_baro_raw_fft    = (23 in selected_options) or all_raw or all_fft
        do_baro_raw_time   = (24 in selected_options) or all_raw or all_time
        do_baro_filt_hist  = (25 in selected_options) or all_filt or all_hist
        do_baro_filt_fft   = (26 in selected_options) or all_filt or all_fft
        do_baro_filt_time  = (27 in selected_options) or all_filt or all_time

        do_att_time        = (28 in selected_options) or all_time or all_raw or all_filt or all_complete

        do_power_hist      = (29 in selected_options) or all_hist or all_raw or all_complete
        do_power_time      = (30 in selected_options) or all_time or all_raw or all_complete

        print(f"\n[INFO] Commencing Analysis with Run ID: {run_id:04d} (Trimming: First {args.trim_start}% and Last {args.trim_end}%)")
        print(f"[INFO] All output images will be saved in: {img_dir}\n")

        # 4. Dispatch tasks
        if (do_accel_raw_hist or do_accel_raw_fft or do_accel_raw_time or
            do_accel_filt_hist or do_accel_filt_fft or do_accel_filt_time):
            process_accelerometer(
                log_data,
                do_accel_raw_hist, do_accel_raw_fft, do_accel_raw_time,
                do_accel_filt_hist, do_accel_filt_fft, do_accel_filt_time,
                run_id, img_dir, args.trim_start, args.trim_end
            )

        if (do_gyro_raw_hist or do_gyro_raw_fft or do_gyro_raw_time or
            do_gyro_filt_hist or do_gyro_filt_fft or do_gyro_filt_time):
            process_gyroscope(
                log_data,
                do_gyro_raw_hist, do_gyro_raw_fft, do_gyro_raw_time,
                do_gyro_filt_hist, do_gyro_filt_fft, do_gyro_filt_time,
                run_id, img_dir, args.trim_start, args.trim_end
            )

        if (do_mag_raw_hist or do_mag_raw_fft or do_mag_raw_time or do_mag_raw_proj or do_mag_raw_calib or
            do_mag_filt_hist or do_mag_filt_fft or do_mag_filt_time or do_mag_filt_proj):
            process_magnetometer(
                log_data,
                do_mag_raw_hist, do_mag_raw_fft, do_mag_raw_time, do_mag_raw_proj, do_mag_raw_calib,
                do_mag_filt_hist, do_mag_filt_fft, do_mag_filt_time, do_mag_filt_proj,
                run_id, img_dir, args.trim_start, args.trim_end
            )

        if (do_baro_raw_hist or do_baro_raw_fft or do_baro_raw_time or
            do_baro_filt_hist or do_baro_filt_fft or do_baro_filt_time):
            process_barometer(
                log_data,
                do_baro_raw_hist, do_baro_raw_fft, do_baro_raw_time,
                do_baro_filt_hist, do_baro_filt_fft, do_baro_filt_time,
                run_id, img_dir, args.trim_start, args.trim_end
            )

        if do_att_time:
            process_attitude(log_data, do_att_time, run_id, img_dir, args.trim_start, args.trim_end)

        if do_power_hist or do_power_time:
            process_power(log_data, do_power_hist, do_power_time, run_id, img_dir, args.trim_start, args.trim_end)

        print(f"\n==========================================================================================")
        print(f"  ANALYSIS COMPLETE! Run ID {run_id:04d} artifacts saved into: {img_dir}")
        print(f"  Session Log Output saved to: {log_file.name} in {log_dir}")
        print(f"==========================================================================================\n")
    finally:
        sys.stdout = orig_stdout
        tee.close()

if __name__ == "__main__":
    main()
