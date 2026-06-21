import numpy as np
import matplotlib.pyplot as plt

# 1. Load the data
FILE_NAME = 'mag_data_time.txt'
try:
    # Based on your pure data sample, there is no header row, so we omit skiprows=1
    time_us, magX, magY, magZ = np.loadtxt(FILE_NAME, delimiter=',', unpack=True)
except FileNotFoundError:
    print(f"Error: Could not find '{FILE_NAME}'. Please check the file path.")
    exit()
except ValueError as e:
    print(f"Error reading data. Details: {e}")
    exit()

# 2. Group the data and titles for easy looping
sensor_data = [
    ("Magnetometer X", magX),
    ("Magnetometer Y", magY),
    ("Magnetometer Z", magZ)
]

# 3. Calculate Sampling Rate
# dt is the average time difference between consecutive samples in seconds
dt_seconds = np.mean(np.diff(time_us)) / 1_000_000.0 
sampling_rate = 1.0 / dt_seconds
n_samples = len(time_us)

print(f"Total valid readings: {n_samples}")
print(f"Calculated True Sampling Rate: {sampling_rate:.2f} Hz")

# 4. Create a figure for the FFT plots (1x3 grid for magnetometer)
fig, axes = plt.subplots(nrows=1, ncols=3, figsize=(16, 5))
fig.suptitle("HMC5883L Data Frequency Spectrum (Discrete FFT)", fontsize=16, fontweight='bold')

# Ensure axes is a flat array to easily iterate over
axes = axes.flatten()

# 5. Loop through the data and plot the FFT
for i, (title, data) in enumerate(sensor_data):
    ax = axes[i]
    
    # Remove the DC offset (mean) so 0 Hz doesn't dominate the plot
    data_zero_mean = data - np.mean(data)
    
    # Compute the 1D Discrete Fourier Transform
    fft_values = np.fft.fft(data_zero_mean)
    frequencies = np.fft.fftfreq(n_samples, d=dt_seconds)
    
    # Isolate the positive half + remove the 0Hz frequency
    half_n = n_samples // 2
    freqs = frequencies[1:half_n]
    
    # Calculate amplitude and convert to Decibels (dB)
    amps = np.abs(fft_values[1:half_n]) * 2.0 / n_samples
    amps_db = 20 * np.log10(np.maximum(amps, 1e-10))
    
    # Plot using a green theme for the magnetometer
    ax.plot(freqs, amps_db, color='#55A868', linewidth=1.0)
    
    # Formatting the subplot
    ax.set_title(title, fontsize=12)
    ax.set_ylabel("Amplitude (dB)")
    ax.set_xlabel("Frequency (Hz)")
    ax.grid(True, alpha=0.3)

# 6. Adjust layout and display/save
plt.tight_layout()
plt.subplots_adjust(top=0.85) 

# Note: You can change this to plt.show() on your local computer to view it interactively!
plt.show()
