import numpy as np
import matplotlib.pyplot as plt

# Initialize lists to store our parsed data
time_ms = []
pressure = []

# 1. Open and read from your local text file
filename = "pressure_data_time_oss0.txt"
try:
    with open(filename, "r") as file:
        for line in file:
            # Skip empty lines or headers if any exist
            if line.strip(): 
                t_val, p_val = line.split(',')
                time_ms.append(float(t_val))
                pressure.append(float(p_val))
except FileNotFoundError:
    print(f"Error: Could not find '{filename}'. Make sure it's in the same folder as this script.")
    exit()

# Convert parsed lists to numpy arrays
time_ms = np.array(time_ms)
pressure = np.array(pressure)
print(np.std(pressure))

# Convert time from milliseconds to seconds
time_s = time_ms / 1000.0
N = len(pressure)

# 2. Dynamically calculate the sampling rate (fs)
dt = np.mean(np.diff(time_s))  # Average time step between readings
fs = 1.0 / dt                  # Sampling frequency
print(f"Sampling freq: {fs} Hz")

# 3. Remove the DC Offset (baseline shift)
pressure_detrended = pressure - np.mean(pressure)

# 4. Compute the Real Fast Fourier Transform (RFFT)
fft_values = np.fft.rfft(pressure_detrended)
frequencies = np.fft.rfftfreq(N, d=dt)

# Normalize amplitude to display real physical units (Pascals)
fft_amplitude = np.abs(fft_values) * 2 / N

# 5. Plot the Results
# Changed: Increased height from 5 to 6 to make room for the top text
plt.figure(figsize=(12, 6))

# Added: A super title that displays the calculated dt and fs values
plt.suptitle(f"Average Sampling Time (dt): {dt:.4f} s  |  Sampling Freq (fs): {fs:.3f} Hz", 
             fontsize=14, fontweight='bold')

# Left Plot: Time Domain
plt.subplot(1, 2, 1)
plt.plot(time_s, pressure_detrended, marker='o', color='b', linestyle='-')
plt.title("Time Domain (DC Offset Removed)")
plt.xlabel("Time (seconds)")
plt.ylabel("Relative Pressure (Pa)")
plt.grid(True)

# Right Plot: Frequency Domain
plt.subplot(1, 2, 2)
plt.stem(frequencies, fft_amplitude, linefmt='r-', markerfmt='ro', basefmt='k-')
plt.title("FFT Frequency Spectrum")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Amplitude (Pa)")
plt.xlim(0, fs / 2)  # Limit to the Nyquist Frequency
plt.grid(True)

# Changed: Added rect=[0, 0, 1, 0.95] to prevent graphs from overlapping the suptitle
plt.tight_layout(rect=[0, 0, 1, 0.95])
plt.show()
