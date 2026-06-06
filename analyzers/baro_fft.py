import numpy as np
import matplotlib.pyplot as plt

# Initialize lists to store our parsed data
time_ms = []
pressure = []

# 1. Open and read from your local text file
filename = "pressure_data_time_oss3.txt"
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

# Convert time from milliseconds to seconds
time_s = time_ms / 1000.0
N = len(pressure)

# 2. Dynamically calculate the sampling rate (fs)
dt = np.mean(np.diff(time_s))  # Average time step between readings
fs = 1.0 / dt                  # Sampling frequency

# 3. Remove the DC Offset (baseline shift)
pressure_detrended = pressure - np.mean(pressure)

# 4. Compute the Real Fast Fourier Transform (RFFT)
fft_values = np.fft.rfft(pressure_detrended)
freqs = np.fft.rfftfreq(N, d=dt)

# Normalize amplitude to display real physical units (Pascals)
amps = np.abs(fft_values) * 2 / N
ampsdB = 20 * np.log10(np.maximum(amps, 1e-10))

# 5. Plot the Results
plt.figure(figsize=(16, 9))
plt.suptitle(f"Avg Sampling Time = {dt:.4f} s  |  Sampling Freq = {fs:.3f} Hz", 
             fontsize=14, fontweight='bold')

# remove the 0Hz frequency amplitude, which is 200
freqs = np.delete(freqs, 0)
ampsdB = np.delete(ampsdB, 0)

# plt.stem(freqs, ampsdB, linefmt='r-', markerfmt='', basefmt='k-')
plt.plot(freqs, ampsdB, color='midnightblue', linewidth=1)
plt.title("FFT Frequency Spectrum")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Amplitude (dB)")
plt.xlim(0, fs / 2)  # Limit to the Nyquist Frequency
plt.ylim(np.min(ampsdB) * 1.1, np.max(ampsdB) * 1.1)  # Limit to the Nyquist Frequency
plt.grid(True)

plt.show()
