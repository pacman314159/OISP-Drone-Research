import numpy as np

# Load data again cleanly
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
            runs[current_run] = (headers, np.array(data))
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
    runs[current_run] = (headers, np.array(data))

# Run 1: PWM, V_batt, Thrust
r1 = runs[1][1]
# Run 4: PWM, V_source, Thrust
r4 = runs[4][1]
# Run 5: PWM, V_source, Thrust
r5 = runs[5][1]
# Run 2: PWM, V_source, Current, Thrust
r2 = runs[2][1]

print("=== 1. DEAD-BAND ANALYSIS ===")
# Points where thrust is 0
zero_thrust_pwm_r4 = r4[r4[:,2] == 0, 0]
print(f"Run 4 zero thrust PWM: {zero_thrust_pwm_r4}")
first_spin_r4 = r4[r4[:,2] > 0, 0][0]
print(f"Run 4 first thrust at PWM: {first_spin_r4} us (Thrust = {r4[r4[:,0]==first_spin_r4, 2][0]} g)")

print("\n=== 2. COMPARISON: BATTERY (RUN 1) vs BENCH SUPPLY (RUN 4 & 5) ===")
# Interpolate or match PWM values
common_pwm = [1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000]
print(f"{'PWM (us)':<10} | {'Run 1 V_bat':<12} | {'Run 1 Thrust':<12} | {'Run 4 V_src':<12} | {'Run 4 Thrust':<12} | {'Thrust Diff':<12}")
print("-" * 75)
for p in common_pwm:
    t1_idx = np.where(r1[:,0] == p)[0]
    t4_idx = np.where(r4[:,0] == p)[0]
    if len(t1_idx) > 0 and len(t4_idx) > 0:
        v1, th1 = r1[t1_idx[0], 1], r1[t1_idx[0], 2]
        v4, th4 = r4[t4_idx[0], 1], r4[t4_idx[0], 2]
        print(f"{p:<10.0f} | {v1:<12.2f} | {th1:<12.0f} | {v4:<12.2f} | {th4:<12.0f} | {th1 - th4:<12.0f}")

print("\n=== 3. EFFECTIVE MOTOR VOLTAGE MODELING ===")
# In Run 4 & 5, V_source is constant 11.73 V.
# Let throttle duty sigma = (PWM - 1060) / (2000 - 1060) for active region (PWM > 1060)
# Or standard ESC pulse ratio sigma = (PWM - 1000) / 1000.
# Effective voltage V_eff = sigma * V_source.

# Filter active data for Run 4:
act_r4 = r4[r4[:,0] >= 1080]
pwm_act = act_r4[:,0]
v_src = act_r4[:,1]
thrust_act = act_r4[:,2]

sigma_std = (pwm_act - 1000.0) / 1000.0
v_eff_std = sigma_std * v_src

# Fit 1: Linear T = k * V_eff
p1 = np.polyfit(v_eff_std, thrust_act, 1)
pred1 = np.polyval(p1, v_eff_std)
r2_1 = 1 - np.sum((thrust_act - pred1)**2) / np.sum((thrust_act - np.mean(thrust_act))**2)

# Fit 2: Pure Quadratic T = k * V_eff^2
k_quad = np.sum(thrust_act * (v_eff_std**2)) / np.sum((v_eff_std**2)**2)
pred2 = k_quad * (v_eff_std**2)
r2_2 = 1 - np.sum((thrust_act - pred2)**2) / np.sum((thrust_act - np.mean(thrust_act))**2)

# Fit 3: 2nd order polynomial T = c2 * V^2 + c1 * V + c0
p2 = np.polyfit(v_eff_std, thrust_act, 2)
pred3 = np.polyval(p2, v_eff_std)
r2_3 = 1 - np.sum((thrust_act - pred3)**2) / np.sum((thrust_act - np.mean(thrust_act))**2)

# Fit 4: 3rd order polynomial T = c3 * V^3 + c2 * V^2 + c1 * V + c0 (Crazyflie model)
p3 = np.polyfit(v_eff_std, thrust_act, 3)
pred4 = np.polyval(p3, v_eff_std)
r2_4 = 1 - np.sum((thrust_act - pred4)**2) / np.sum((thrust_act - np.mean(thrust_act))**2)

# Fit 5: Power law T = a * (V_eff)^b
log_v = np.log(v_eff_std)
log_t = np.log(thrust_act)
p_pow = np.polyfit(log_v, log_t, 1)
b_pow = p_pow[0]
a_pow = np.exp(p_pow[1])
pred5 = a_pow * (v_eff_std**b_pow)
r2_5 = 1 - np.sum((thrust_act - pred5)**2) / np.sum((thrust_act - np.mean(thrust_act))**2)

print(f"Model Fits for Run 4 (Thrust [g] vs V_eff [V]):")
print(f"  Linear: R^2 = {r2_1:.5f}")
print(f"  Pure Quadratic (T = k * V^2): k = {k_quad:.3f}, R^2 = {r2_2:.5f}")
print(f"  2nd-order poly: {p2[0]:.3f}*V^2 + {p2[1]:.3f}*V + {p2[2]:.3f}, R^2 = {r2_3:.5f}")
print(f"  3rd-order poly: {p3[0]:.4f}*V^3 + {p3[1]:.3f}*V^2 + {p3[2]:.3f}*V + {p3[3]:.3f}, R^2 = {r2_4:.5f}")
print(f"  Power law T = a * V^b: a = {a_pow:.3f}, exponent b = {b_pow:.3f}, R^2 = {r2_5:.5f}")

# Check Run 1 (Battery) with this V_eff model!
# In Run 1, V_batt is sagging!
act_r1 = r1[r1[:,0] >= 1100]
pwm_r1 = act_r1[:,0]
v_batt_r1 = act_r1[:,1]
thrust_r1 = act_r1[:,2]

sigma_r1 = (pwm_r1 - 1000.0) / 1000.0
v_eff_r1 = sigma_r1 * v_batt_r1

# Predict thrust using 2nd order and 3rd order fit from Run 4:
pred_r1_from_r4_poly2 = np.polyval(p2, v_eff_r1)
pred_r1_from_r4_poly3 = np.polyval(p3, v_eff_r1)
r2_r1_cross = 1 - np.sum((thrust_r1 - pred_r1_from_r4_poly2)**2) / np.sum((thrust_r1 - np.mean(thrust_r1))**2)
print(f"\nGeneralization Test on Battery Run 1 using Model trained on Bench Supply Run 4:")
print(f"  Cross-validation R^2 = {r2_r1_cross:.5f}!")

