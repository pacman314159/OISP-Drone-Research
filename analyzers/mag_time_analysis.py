import numpy as np
import matplotlib.pyplot as plt

FILE_NAME = 'mag_data_time.txt'

# 1. Load the data
try:
    time_us, magX, magY, magZ = np.loadtxt(FILE_NAME, delimiter=',', unpack=True)
except FileNotFoundError:
    print(f"Error: Could not find '{FILE_NAME}'. Please check the file path.")
    exit()
except ValueError as e:
    print(f"Error reading data. Ensure the file has exactly 4 columns separated by commas.\nDetails: {e}")
    exit()

# 2. Filter out consecutive duplicate readings
mag_matrix = np.column_stack((magX, magY, magZ))
diffs = np.diff(mag_matrix, axis=0)
is_not_duplicate = np.any(diffs != 0, axis=1)
mask = np.insert(is_not_duplicate, 0, True)

time_us_clean = time_us[mask]
magX_clean = magX[mask]
magY_clean = magY[mask]
magZ_clean = magZ[mask]

# 3. Convert absolute microseconds to relative seconds
# This makes the X-axis start at 0.0s and count upwards, which is much easier to read
time_s = (time_us_clean - time_us_clean[0]) / 1_000_000.0

# 4. Create a figure with 3 vertically stacked subplots sharing the X-axis
fig, axes = plt.subplots(nrows=3, ncols=1, figsize=(14, 8), sharex=True)
fig.suptitle("HMC5883L Magnetometer Readings Over Time", fontsize=16, fontweight='bold')

# Plot X-Axis
axes[0].plot(time_s, magX_clean, color='#D55E00', linewidth=1.5, label='X-Axis')
axes[0].set_ylabel("Gauss")
axes[0].legend(loc='upper right')
axes[0].grid(True, linestyle='--', alpha=0.6)

# Plot Y-Axis
axes[1].plot(time_s, magY_clean, color='#0072B2', linewidth=1.5, label='Y-Axis')
axes[1].set_ylabel("Gauss")
axes[1].legend(loc='upper right')
axes[1].grid(True, linestyle='--', alpha=0.6)

# Plot Z-Axis
axes[2].plot(time_s, magZ_clean, color='#009E73', linewidth=1.5, label='Z-Axis')
axes[2].set_xlabel("Time (seconds)")
axes[2].set_ylabel("Gauss")
axes[2].legend(loc='upper right')
axes[2].grid(True, linestyle='--', alpha=0.6)

# Finalize layout and show
plt.tight_layout()
plt.subplots_adjust(top=0.92) # Leave space for the main title
plt.show()
