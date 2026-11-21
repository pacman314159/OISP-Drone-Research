import numpy as np

# Load Run 2
with open("thrust_bench_data.csv", "r") as f:
    lines = f.readlines()

data2 = []
is_r2 = False
for line in lines:
    if "Test No,2" in line:
        is_r2 = True
        continue
    if is_r2 and "Test No,3" in line:
        break
    if is_r2 and "PWM" not in line and line.strip():
        parts = [p.strip() for p in line.split(",") if p.strip()]
        if len(parts) >= 4:
            data2.append([float(x) for x in parts[:4]])

r2 = np.array(data2)
pwm = r2[:,0]
v_src = r2[:,1]
curr = r2[:,2]
thrust = r2[:,3]

power = v_src * curr
eff = np.zeros_like(thrust)
mask = power > 0
eff[mask] = thrust[mask] / power[mask] # g/W

print(f"{'PWM (us)':<10} | {'Current (A)':<12} | {'Power (W)':<12} | {'Thrust (g)':<12} | {'Efficiency (g/W)':<15}")
print("-" * 70)
for i in range(0, len(pwm), 3):
    print(f"{pwm[i]:<10.0f} | {curr[i]:<12.2f} | {power[i]:<12.2f} | {thrust[i]:<12.0f} | {eff[i]:<15.2f}")

