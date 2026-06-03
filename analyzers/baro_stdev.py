import numpy as np
import matplotlib.pyplot as plt

# Formula constants from BMP180 Datasheet
P0 = 101325.0  # Standard sea level pressure in Pascals

# 1. Load data from the file
try:
    time_ms, pressure_pa = np.loadtxt('pressure_data_time_oss3.txt', delimiter=',', unpack=True)
except FileNotFoundError:
    print("Error: 'pressure_data_time.txt' not found. Ensure it is in the same folder as this script.")
    exit()

# 2. Convert Pressure to Altitude (meters)
altitude = 44330.0 * (1.0 - (pressure_pa / P0)**(1.0 / 5.255))

# 3. Calculate Statistics for both metrics
mean_press = np.mean(pressure_pa)
std_press = np.std(pressure_pa)

mean_alt = np.mean(altitude)
std_alt = np.std(altitude)

# print(f"--- Pressure Analysis ---")
# print(f"Mean Pressure: {mean_press:.2f} Pa")
# print(f"Std Deviation: {std_press:.2f} Pa\n")
#
# print(f"--- Altitude Analysis ---")
# print(f"Mean Altitude: {mean_alt:.2f} meters")
# print(f"Std Deviation: {std_alt:.2f} meters")

# 4. Plotting Side-by-Side Histograms
plt.figure(figsize=(14, 5))

# Dynamically set a lower bin count if dealing with smaller logging files
num_bins = 15 if len(pressure_pa) < 100 else 50

# --- Left Plot: Pressure Histogram ---
plt.subplot(1, 2, 1)
plt.hist(pressure_pa, bins=num_bins, color='skyblue', edgecolor='black', alpha=0.8)
plt.axvline(mean_press, color='red', linestyle='--', label=f'Mean: {mean_press:.1f} Pa')
# Added: Standard Deviation lines for Pressure
plt.axvline(mean_press + std_press, color='purple', linestyle=':', linewidth=2, label=f'SD: {std_press:.3f} Pa')
plt.axvline(mean_press - std_press, color='purple', linestyle=':', linewidth=2, label='')
plt.title('Histogram of Measured Pressure', fontsize=14)
plt.xlabel('Pressure (Pascals)', fontsize=12)
plt.ylabel('Frequency', fontsize=12)
plt.legend()
plt.grid(axis='y', alpha=0.3)

# --- Right Plot: Altitude Histogram ---
plt.subplot(1, 2, 2)
plt.hist(altitude, bins=num_bins, color='lightgreen', edgecolor='black', alpha=0.8)
plt.axvline(mean_alt, color='red', linestyle='--', label=f'Mean: {mean_alt:.3f}m')
# Added: Standard Deviation lines for Altitude
plt.axvline(mean_alt + std_alt, color='purple', linestyle=':', linewidth=2, label=f'SD: {std_alt:.3f} m')
plt.axvline(mean_alt - std_alt, color='purple', linestyle=':', linewidth=2, label='')
plt.title('Histogram of Estimated Altitude', fontsize=14)
plt.xlabel('Altitude (meters)', fontsize=12)
plt.ylabel('Frequency', fontsize=12)
plt.legend()
plt.grid(axis='y', alpha=0.3)

plt.tight_layout()
plt.show()
