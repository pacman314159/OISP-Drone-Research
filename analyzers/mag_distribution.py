import numpy as np
import matplotlib.pyplot as plt

FILE_NAME = 'mag_data_time.txt'

try:
    time_us, magX, magY, magZ = np.loadtxt(FILE_NAME, delimiter=',', unpack=True)
except FileNotFoundError:
    print(f"Error: Could not find '{FILE_NAME}'. Please check the file path.")
    exit()
except ValueError as e:
    print(f"Error reading data. Ensure the file has exactly 4 columns separated by commas.\nDetails: {e}")
    exit()

# Filter out consecutive duplicate readings
mag_matrix = np.column_stack((magX, magY, magZ))
diffs = np.diff(mag_matrix, axis=0)
is_not_duplicate = np.any(diffs != 0, axis=1)
mask = np.insert(is_not_duplicate, 0, True)

time_us_clean = time_us[mask]
magX_clean = magX[mask]
magY_clean = magY[mask]
magZ_clean = magZ[mask]

print(f"Original readings: {len(magX)}")
print(f"Cleaned readings:  {len(magX_clean)} (Removed {len(magX) - len(magX_clean)} consecutive duplicates)")

# Calculate and print the real sampling frequency
time_intervals_us = np.diff(time_us_clean)
avg_interval_us = np.mean(time_intervals_us)
real_sampling_freq = 1_000_000.0 / avg_interval_us

print(f"Real Sampling Frequency: {real_sampling_freq:.6f} Hz")

sensor_data = [
    ("Magnetometer X (Gauss)", magX_clean),
    ("Magnetometer Y (Gauss)", magY_clean),
    ("Magnetometer Z (Gauss)", magZ_clean)
]

fig, axes = plt.subplots(nrows=1, ncols=3, figsize=(16, 5))
fig.suptitle("HMC5883L Data Distributions (Cleaned)", fontsize=16, fontweight='bold')

for i, (title, data) in enumerate(sensor_data):
    ax = axes[i]
    mean_val = np.mean(data)
    std_val = np.std(data)
    
    ax.hist(data, bins=50, alpha=0.7, color='#55A868', edgecolor='black') 
    ax.axvline(mean_val, color='red', linestyle='solid', linewidth=2, label=f'Mean: {mean_val:.6f}')
    ax.axvline(mean_val - std_val, color='orange', linestyle='dashed', linewidth=2, label=f'SD: {std_val:.6f}')
    ax.axvline(mean_val + std_val, color='orange', linestyle='dashed', linewidth=2, label='')
    
    ax.set_title(title, fontsize=12)
    ax.set_ylabel("Frequency")
    ax.set_xlabel("Sensor Value")
    ax.legend(loc='upper right', fontsize=9)
    ax.grid(axis='y', alpha=0.3)

plt.tight_layout()
plt.subplots_adjust(top=0.85)
plt.show()
