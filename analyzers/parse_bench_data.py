import pandas as pd
import numpy as np

# Parse the CSV into distinct test runs
with open("thrust_bench_data.csv", "r") as f:
    lines = f.readlines()

runs = {}
current_run = None
headers = []
data = []

for line in lines:
    line_str = line.strip()
    if not line_str or line_str.replace(",", "") == "":
        continue
    parts = [p.strip() for p in line_str.split(",")]
    if parts[0] == "Test No":
        if current_run is not None and data:
            runs[current_run] = (headers, data)
        current_run = int(parts[1])
        headers = []
        data = []
    elif "PWM" in parts[0]:
        headers = parts
    elif current_run is not None:
        if parts[0] == "Cut off" or "fried" in line_str:
            continue
        try:
            row = [float(p) for p in parts if p != ""]
            if len(row) >= 3:
                data.append(row)
        except ValueError:
            pass

if current_run is not None and data:
    runs[current_run] = (headers, data)

print(f"Loaded {len(runs)} runs: {list(runs.keys())}")
for r_id, (h, d) in runs.items():
    print(f"Run {r_id}: {h} -> {len(d)} rows")

# Let's inspect Run 1 (Battery), Run 4 (Bench supply full sweep), Run 5 (Bench supply full sweep)
# Run 1: PWM, V_batt, Thrust (g)
d1 = np.array(runs[1][1])
# Run 4: PWM, V_source, Thrust (g)
d4 = np.array(runs[4][1])
# Run 5: PWM, V_source, Thrust (g)
d5 = np.array(runs[5][1])

print("\n--- RUN 1 (Battery) sample ---")
print("PWM min/max:", d1[:,0].min(), d1[:,0].max())
print("V_batt min/max:", d1[:,1].min(), d1[:,1].max())
print("Thrust min/max:", d1[:,2].min(), d1[:,2].max())

print("\n--- RUN 4 (Bench Supply) sample ---")
print("PWM min/max:", d4[:,0].min(), d4[:,0].max())
print("V_source:", d4[0,1])
print("Thrust min/max:", d4[:,2].min(), d4[:,2].max())

