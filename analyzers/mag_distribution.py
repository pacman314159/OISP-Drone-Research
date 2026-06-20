import numpy as np
import matplotlib.pyplot as plt

# 1. Load the data
FILE_NAME = 'mag_data_time.txt'
try:
    # Reading your perfectly synced 4-column data
    time_us, magX, magY, magZ = np.loadtxt(FILE_NAME, delimiter=',', unpack=True)
except FileNotFoundError:
    print(f"Error: Could not find '{FILE_NAME}'. Please check the file path.")
    exit()
except ValueError as e:
    print(f"Error reading data. Ensure the file has exactly 4 columns.\nDetails: {e}")
    exit()

# 2. Calculate the Sampling Intervals (in microseconds)
# np.diff subtracts each timestamp from the one after it
# We no longer divide by 1000, keeping the unit in microseconds (µs)
intervals_us = np.diff(time_us)
mean_interval = np.mean(intervals_us)
std_interval = np.std(intervals_us)

print(f"Total readings analyzed: {len(time_us)}")
print(f"Average Sampling Interval: {mean_interval:.2f} µs (approx. {1_000_000 / mean_interval:.2f} Hz)")
print(f"Interval Standard Deviation (Jitter): ±{std_interval:.2f} µs")

# 3. Group the magnetometer data and titles
sensor_data = [
    ("Magnetometer X (Gauss)", magX),
    ("Magnetometer Y (Gauss)", magY),
    ("Magnetometer Z (Gauss)", magZ)
]

# 4. Create a figure with a 2x2 grid of subplots
fig, axes = plt.subplots(nrows=2, ncols=2, figsize=(14, 10))
fig.suptitle("HMC5883L Data Distributions & Sampling Jitter", fontsize=16, fontweight='bold')

# Flatten the 2x2 grid into a 1D array so we can easily index 0, 1, 2, and 3
axes = axes.flatten()

# 5. Loop through the X, Y, and Z data and plot the first 3 histograms
for i, (title, data) in enumerate(sensor_data):
    ax = axes[i]
    mean_val = np.mean(data)
    std_val = np.std(data)
    
    # Green color for the magnetic axes
    ax.hist(data, bins=50, alpha=0.7, color='#55A868', edgecolor='black')
    
    ax.axvline(mean_val, color='red', linestyle='solid', linewidth=2, 
               label=f'Mean: {mean_val:.6f}')
    ax.axvline(mean_val - std_val, color='orange', linestyle='dashed', linewidth=2, 
               label=f'SD: {std_val:.6f}')
    ax.axvline(mean_val + std_val, color='orange', linestyle='dashed', linewidth=2)
    
    ax.set_title(title, fontsize=12)
    ax.set_ylabel("Frequency")
    ax.set_xlabel("Sensor Value")
    ax.legend(loc='upper right', fontsize=9)
    ax.grid(axis='y', alpha=0.3)

# 6. Plot the Sampling Interval Variation in the 4th slot
ax4 = axes[3]

# Using a distinct purple color for timing data
ax4.hist(intervals_us, bins=50, alpha=0.7, color='#9467BD', edgecolor='black')

ax4.axvline(mean_interval, color='red', linestyle='solid', linewidth=2, 
            label=f'Mean: {mean_interval:.0f} µs')
ax4.axvline(mean_interval - std_interval, color='orange', linestyle='dashed', linewidth=2, 
            label=f'SD: {std_interval:.2f} µs')
ax4.axvline(mean_interval + std_interval, color='orange', linestyle='dashed', linewidth=2)

ax4.set_title("Sampling Interval Variation (Jitter)", fontsize=12)
ax4.set_ylabel("Frequency")
ax4.set_xlabel("Time Between Readings (microseconds)")
ax4.legend(loc='upper right', fontsize=9)
ax4.grid(axis='y', alpha=0.3)

# 7. Finalize layout and display
plt.tight_layout()
plt.subplots_adjust(top=0.92) 
plt.show()
