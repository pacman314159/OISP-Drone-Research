#!/usr/bin/env python3
"""
BLE OTA Firmware Uploader for FC_v01 / FC_v02 Flight Controller
Supports both standalone CLI usage and PlatformIO custom upload target.

The FC OTA listener task runs for 10 seconds upon boot-up.
Run this script within 10 seconds of FC reboot/power-on.
"""

import sys
import os
import time
import struct
import binascii
import asyncio
import argparse
from pathlib import Path

try:
    from bleak import BleakClient, BleakScanner
except ImportError:
    print("Error: 'bleak' package is required. Install via: pip install bleak", file=sys.stderr)
    sys.exit(1)

# Centralized Service & Characteristic UUIDs matching include/config.h
BLE_SERVICE_OTA_UUID      = "23408888-1f40-4cd8-9b89-ca8d45f8a5b0"
BLE_CHAR_OTA_CONTROL_UUID = "23408889-1f40-4cd8-9b89-ca8d45f8a5b0"
BLE_CHAR_OTA_DATA_UUID    = "2340888a-1f40-4cd8-9b89-ca8d45f8a5b0"
DEFAULT_DEVICE_NAME       = "FC_v02"

# Protocol OpCodes
CMD_OTA_START = 0x01
CMD_OTA_END   = 0x02
CMD_OTA_ABORT = 0x03

RESP_OK         = 0x00
RESP_READY_DATA = 0x01
RESP_PROGRESS   = 0x02
RESP_SUCCESS    = 0x03
RESP_ERR_SIZE   = 0x81
RESP_ERR_FLASH  = 0x82
RESP_ERR_VERIFY = 0x83
RESP_ERR_ABORT  = 0x84

DEFAULT_CHUNK_SIZE = 490  # Max payload per WRITE_NR at MTU 512
DEFAULT_WINDOW_SIZE = 8   # Chunks pipelined via WRITE_NR before requiring ACK (8 chunks ~ 3.9 KB aligned with 4 KB flash sector)


def find_firmware_binary(arg_path=None):
    if arg_path and os.path.isfile(arg_path):
        return os.path.abspath(arg_path)

    project_root = Path(__file__).resolve().parent.parent
    candidate_locations = [
        project_root / ".pio" / "build" / "fc-v01-ota" / "firmware.bin",
        project_root / ".pio" / "build" / "fc-v01" / "firmware.bin",
        Path(os.getcwd()) / ".pio" / "build" / "fc-v01-ota" / "firmware.bin",
        Path(os.getcwd()) / ".pio" / "build" / "fc-v01" / "firmware.bin",
    ]

    for cand in candidate_locations:
        if cand.is_file():
            return str(cand.resolve())

    return None


class BleOtaUploader:
    def __init__(self, bin_path, device_name=DEFAULT_DEVICE_NAME, chunk_size=DEFAULT_CHUNK_SIZE, window_size=DEFAULT_WINDOW_SIZE):
        self.bin_path = bin_path
        self.device_name = device_name
        self.chunk_size = chunk_size
        self.window_size = max(1, window_size)
        self.ready_event = asyncio.Event()
        self.finish_event = asyncio.Event()
        self.error_occurred = False
        self.error_message = ""
        self.fc_bytes_written = 0
        self.fw_size = 0

    def notification_handler(self, sender, data: bytearray):
        if not data:
            return
        code = data[0]

        if code == RESP_READY_DATA:
            self.ready_event.set()
        elif code == RESP_PROGRESS:
            if len(data) >= 6:
                _, written, _ = struct.unpack("<BIB", data[:6])
                self.fc_bytes_written = written
        elif code == RESP_SUCCESS:
            self.finish_event.set()
        elif code == RESP_ERR_SIZE:
            self.error_occurred = True
            self.error_message = "Firmware binary size exceeds partition capacity."
            self.ready_event.set()
            self.finish_event.set()
        elif code == RESP_ERR_FLASH:
            self.error_occurred = True
            self.error_message = "Flash write/erase error reported by Flight Controller."
            self.ready_event.set()
            self.finish_event.set()
        elif code == RESP_ERR_VERIFY:
            self.error_occurred = True
            if self.fw_size > 0 and self.fc_bytes_written < self.fw_size:
                self.error_message = (
                    f"Image verification failed on Flight Controller. "
                    f"Size mismatch: FC confirmed {self.fc_bytes_written:,}/{self.fw_size:,} bytes written "
                    f"({self.fw_size - self.fc_bytes_written:,} bytes dropped during transmission)."
                )
            else:
                self.error_message = "Image verification failed on Flight Controller (esp_ota_end image integrity check failed)."
            self.ready_event.set()
            self.finish_event.set()
        elif code == RESP_ERR_ABORT:
            self.error_occurred = True
            self.error_message = "OTA session aborted."
            self.ready_event.set()
            self.finish_event.set()

    async def run(self):
        with open(self.bin_path, "rb") as f:
            fw_data = f.read()

        fw_size = len(fw_data)
        self.fw_size = fw_size
        crc32_val = binascii.crc32(fw_data) & 0xFFFFFFFF
        print(f"[*] Firmware Binary: {self.bin_path}")
        print(f"[*] Image Size: {fw_size:,} bytes ({fw_size / 1024:.2f} KB)")
        print(f"[*] CRC32: 0x{crc32_val:08X}")
        print(f"[*] Scanning for BLE device '{self.device_name}' (listening window is 10s after FC boot)...")

        device = None
        scan_attempts = 0
        while not device and scan_attempts < 3:
            scan_attempts += 1
            device = await BleakScanner.find_device_by_filter(
                lambda d, adv: (d.name and self.device_name in d.name) or (adv.local_name and self.device_name in adv.local_name),
                timeout=4.0
            )
            if not device and scan_attempts < 3:
                print(f"[-] Not found yet, retrying scan ({scan_attempts}/3)...")

        if not device:
            print("\n[!] Error: Flight Controller not found!", file=sys.stderr)
            print("[!] Remember: The OTA window only listens for 10 seconds after FC boot/power-on.", file=sys.stderr)
            print("[!] Please power-cycle / reset the FC, and run this upload immediately.", file=sys.stderr)
            return False

        print(f"[+] Found FC device: {device.name} ({device.address})")
        print("[*] Connecting via BLE...")

        async with BleakClient(device, timeout=10.0) as client:
            if not client.is_connected:
                print("[!] Error: Failed to connect to BLE device.", file=sys.stderr)
                return False

            print("[+] Connected to Flight Controller!")

            # Dynamically clamp chunk size to negotiated ATT MTU if supported
            if hasattr(client, "mtu_size") and client.mtu_size > 23:
                max_payload = max(20, client.mtu_size - 3)
                if self.chunk_size > max_payload:
                    print(f"[*] Clamping chunk size from {self.chunk_size} to {max_payload} bytes based on negotiated MTU ({client.mtu_size} B).")
                    self.chunk_size = max_payload
                else:
                    print(f"[*] Negotiated ATT MTU: {client.mtu_size} bytes (using chunk size: {self.chunk_size} bytes).")

            # Subscribe to Control Characteristic notifications
            await client.start_notify(BLE_CHAR_OTA_CONTROL_UUID, self.notification_handler)
            await asyncio.sleep(0.1)

            # Send CMD_OTA_START
            start_payload = struct.pack("<BII", CMD_OTA_START, fw_size, crc32_val)
            print("[*] Sending OTA Handshake / Start command...")
            await client.write_gatt_char(BLE_CHAR_OTA_CONTROL_UUID, start_payload, response=True)

            # Wait for FC to initialize partition and respond with RESP_READY_DATA
            try:
                await asyncio.wait_for(self.ready_event.wait(), timeout=15.0)
            except asyncio.TimeoutError:
                print("[!] Error: Timed out waiting for FC ready ACK.", file=sys.stderr)
                return False

            if self.error_occurred:
                print(f"[!] Error: {self.error_message}", file=sys.stderr)
                return False

            print(f"[+] Partition ready! All flight tasks suspended. Streaming firmware chunks (window: {self.window_size})...")
            total_chunks = (fw_size + self.chunk_size - 1) // self.chunk_size
            start_time = time.time()
            bytes_sent = 0

            try:
                for i in range(total_chunks):
                    chunk_offset = i * self.chunk_size
                    chunk = fw_data[chunk_offset:chunk_offset + self.chunk_size]

                    # Windowed pipeline: stream via WRITE_NR (response=False) and enforce
                    # backpressure flow control with response=True on window boundaries and at the final chunk.
                    is_last_chunk = (i == total_chunks - 1)
                    is_window_boundary = ((i + 1) % self.window_size == 0)
                    use_response = is_last_chunk or is_window_boundary or (self.window_size == 1)

                    await client.write_gatt_char(BLE_CHAR_OTA_DATA_UUID, chunk, response=use_response)
                    bytes_sent += len(chunk)

                    # Progress bar
                    pct = (bytes_sent / fw_size) * 100
                    elapsed = time.time() - start_time
                    speed_kb = (bytes_sent / 1024) / elapsed if elapsed > 0 else 0
                    bar_len = 30
                    filled = int(bar_len * (bytes_sent / fw_size))
                    bar = "=" * filled + "-" * (bar_len - filled)
                    fc_info = f" | Flashed: {self.fc_bytes_written:,} B" if self.fc_bytes_written > 0 else ""
                    sys.stdout.write(f"\r[{bar}] {pct:5.1f}% | {bytes_sent:,}/{fw_size:,} B{fc_info} | {speed_kb:5.1f} KB/s")
                    sys.stdout.flush()

                    if self.error_occurred:
                        print(f"\n[!] Error during transfer: {self.error_message}", file=sys.stderr)
                        return False

                print()
                elapsed_total = time.time() - start_time
                print(f"[+] All chunks transmitted in {elapsed_total:.2f}s (Average: {(fw_size / 1024) / elapsed_total:.1f} KB/s).")
                print("[*] Sending OTA End command and awaiting flash verification...")

                end_payload = struct.pack("<BI", CMD_OTA_END, fw_size)
                await client.write_gatt_char(BLE_CHAR_OTA_CONTROL_UUID, end_payload, response=True)

                try:
                    await asyncio.wait_for(self.finish_event.wait(), timeout=15.0)
                except asyncio.TimeoutError:
                    print("[!] Error: Timed out waiting for verification/reboot response.", file=sys.stderr)
                    return False

            except (KeyboardInterrupt, asyncio.CancelledError):
                print("\n[!] Transfer aborted by user (Ctrl+C)!")
                print("[*] Sending CMD_OTA_ABORT to Flight Controller...")
                try:
                    abort_payload = struct.pack("<B", CMD_OTA_ABORT)
                    await client.write_gatt_char(BLE_CHAR_OTA_CONTROL_UUID, abort_payload, response=False)
                    await asyncio.sleep(0.2)
                except Exception:
                    pass
                print("[*] Abort signal dispatched. Flight Controller is safely rolling back.")
                return False

            if self.error_occurred:
                print(f"[!] Update Failed: {self.error_message}", file=sys.stderr)
                return False

            print("\n" + "=" * 60)
            print("[SUCCESS] Firmware verified and flashed successfully!")
            print("[SUCCESS] Flight Controller is now rebooting into the new image.")
            print("=" * 60)
            return True


def main():
    parser = argparse.ArgumentParser(description="Upload firmware to FC_v01/FC_v02 via BLE OTA")
    parser.add_argument("binary", nargs="?", help="Path to firmware.bin (optional, will auto-detect if omitted)")
    parser.add_argument("--bin", dest="bin_flag", help="Alternative flag for firmware binary path")
    parser.add_argument("--name", default=DEFAULT_DEVICE_NAME, help=f"BLE device name (default: {DEFAULT_DEVICE_NAME})")
    parser.add_argument("--chunk", type=int, default=DEFAULT_CHUNK_SIZE, help=f"Chunk size in bytes (default: {DEFAULT_CHUNK_SIZE})")
    parser.add_argument("--window", type=int, default=DEFAULT_WINDOW_SIZE, help=f"Pipelined chunk window size before requiring ACK (default: {DEFAULT_WINDOW_SIZE}, set 1 for unpipelined)")

    args = parser.parse_args()
    bin_arg = args.bin_flag if args.bin_flag else args.binary
    target_bin = find_firmware_binary(bin_arg)

    if not target_bin:
        print("Error: Could not locate firmware.bin. Specify path explicitly: python tools/ble_ota_uploader.py <firmware.bin>", file=sys.stderr)
        sys.exit(1)

    uploader = BleOtaUploader(target_bin, device_name=args.name, chunk_size=args.chunk, window_size=args.window)
    try:
        success = asyncio.run(uploader.run())
    except KeyboardInterrupt:
        print("\n[!] Program interrupted by user. Exiting.")
        sys.exit(130)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
