import numpy as np
import matplotlib.pyplot as plt

# 1. Load the data
FILE_NAME = 'imu_data_time.txt'
try:
    time_us, accX, accY, accZ, gyroX, gyroY, gyroZ, temp = np.loadtxt(FILE_NAME, delimiter=',', unpack=True, skiprows=1)
except FileNotFoundError:
    print(f"Error: Could not find '{FILE_NAME}'. Please check the file path.")
    exit()

# 2. Group the data and titles
sensor_data = [
    ("Accelerometer X", accX),
    ("Accelerometer Y", accY),
    ("Accelerometer Z", accZ),
    ("Gyroscope X", gyroX),
    ("Gyroscope Y", gyroY),
    ("Gyroscope Z", gyroZ)
]

# 3. Calculate Sampling Rate
# dt is the time difference between samples in seconds
dt_seconds = np.mean(np.diff(time_us)) / 1_000_000.0 
sampling_rate = 1.0 / dt_seconds
n_samples = len(time_us)

print(f"Calculated Sampling Rate: {sampling_rate:.2f} Hz")

# 4. Create a figure for the FFT plots
fig, axes = plt.subplots(nrows=2, ncols=3, figsize=(16, 10))
fig.suptitle("IMU Data Frequency Spectrum (Discrete FFT Bins)", fontsize=16, fontweight='bold')
axes = axes.flatten()

# 5. Loop through the data and plot the FFT
for i, (title, data) in enumerate(sensor_data):
    ax = axes[i]
    
    # Remove the DC offset (mean) so 0 Hz doesn't dominate the plot
    data_zero_mean = data - np.mean(data)
    
    # Compute the 1D Discrete Fourier Transform
    fft_values = np.fft.fft(data_zero_mean)
    frequencies = np.fft.fftfreq(n_samples, d=dt_seconds)
    
    # Isolate  positive half + remove 0Hz frequency
    half_n = n_samples // 2
    freqs = frequencies[1:half_n]
    amps = np.abs(fft_values[1:half_n]) * 2.0 / n_samples
    amps_db = 20 * np.log10(np.maximum(amps, 1e-10))
    
    ax.plot(freqs, amps_db, color='midnightblue', linewidth=0.3)
    
    # Formatting the subplot
    ax.set_title(title, fontsize=12)
    ax.set_ylabel("Amplitude (dB)")
    ax.set_xlabel("Frequency (Hz)")
    ax.grid(True, alpha=0.3)

# 6. Adjust layout and display
plt.tight_layout()
plt.subplots_adjust(top=0.92) 
plt.show()
