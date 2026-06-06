import numpy as np
import matplotlib.pyplot as plt

# 1. Load the data
FILE_NAME = 'imu_data_time.txt'
try:
    time_us, accX, accY, accZ, gyroX, gyroY, gyroZ, temp = np.loadtxt(FILE_NAME, delimiter=',', unpack=True, skiprows=1)
except FileNotFoundError:
    print(f"Error: Could not find '{FILE_NAME}'. Please check the file path.")
    exit()

# 2. Group the data and titles for easy looping
sensor_data = [
    ("Accelerometer X (g)", accX),
    ("Accelerometer Y (g)", accY),
    ("Accelerometer Z (g)", accZ),
    ("Gyroscope X (deg/s)", gyroX),
    ("Gyroscope Y (deg/s)", gyroY),
    ("Gyroscope Z (deg/s)", gyroZ)
]

# 3. Create a figure with a 2x3 grid of subplots
fig, axes = plt.subplots(nrows=2, ncols=3, figsize=(16, 10))
fig.suptitle("IMU Data Distributions with Standard Deviations", fontsize=16, fontweight='bold')

# Flatten the 2D axes array to easily iterate over it
axes = axes.flatten()

# 4. Loop through the data and plot each histogram
for i, (title, data) in enumerate(sensor_data):
    ax = axes[i]
    mean_val = np.mean(data)
    std_val = np.std(data)
    
    ax.hist(data, bins=50, alpha=0.7, color='#4C72B0', edgecolor='black')
    ax.axvline(mean_val, color='red', linestyle='solid', linewidth=2, 
               label=f'Mean: {mean_val:.6f}')
    ax.axvline(mean_val - std_val, color='orange', linestyle='dashed', linewidth=2, 
               label=f'SD: {std_val:.6f}')
    ax.axvline(mean_val + std_val, color='orange', linestyle='dashed', linewidth=2, 
               label='')
    
    ax.set_title(title, fontsize=12)
    ax.set_ylabel("Frequency")
    ax.set_xlabel("Sensor Value")
    ax.legend(loc='upper right', fontsize=9)
    ax.grid(axis='y', alpha=0.3)

plt.tight_layout()
plt.subplots_adjust(top=0.92) 
plt.show()
