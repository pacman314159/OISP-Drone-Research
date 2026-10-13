#!/usr/bin/env python3
"""
Generates an annotated slice-by-slice timeline plot of ACCEL_GYRO_DAQ
during its maximum interrupted execution (Pass #1: 3.347 ms -> 5.040 ms).
"""

import os
import matplotlib.pyplot as plt
import matplotlib.patches as patches

output_path = r"D:\OneDrive_MSFT\Work\Drone_Research\analyzers\accel_gyro_interrupted_slice.png"

fig, ax = plt.subplots(figsize=(16, 8))

# Define timeline intervals (start_ms, duration_ms, context_name, color, label)
segments = [
    # ACCEL_GYRO_DAQ wakes up & attempts mutex
    (3.347, 0.056, "ACCEL_GYRO_DAQ", "#1f77b4", "ACCEL_GYRO_DAQ: Wakeup & Mutex Request"),
    # Mutex Blocked: MAG_DAQ has priority inheritance
    (3.403, 0.063, "BLOCKED (Lock Wait)", "#d62728", "BLOCKED: i2c0_mutex held by MAG_DAQ\n(Priority Inherited to MAG_DAQ)"),
    (3.466, 0.173, "MAG_DAQ", "#2ca02c", "MAG_DAQ: Running with inherited Prio 3"),
    (3.639, 0.051, "ISRs (IPC & I2C)", "#ff7f0e", "ISRs: FROM_CPU_INTR0 & I2C_EXT0"),
    (3.690, 0.254, "MAG_DAQ", "#2ca02c", "MAG_DAQ: I2C transaction"),
    (3.944, 0.126, "btController", "#9467bd", "btController: Preempts (Priority 23)"),
    (4.070, 0.068, "MAG_DAQ", "#2ca02c", "MAG_DAQ: Finishes I2C"),
    (4.138, 0.071, "Timer ISRs", "#ff7f0e", "ISRs: SYSTIMER & SysTick"),
    (4.209, 0.058, "MAG_DAQ Release", "#2ca02c", "MAG_DAQ: Disinherit & Give Mutex"),
    
    # ACCEL_GYRO_DAQ gets mutex -> Sub-slice 1
    (4.267, 0.129, "ACCEL_GYRO_DAQ", "#1f77b4", "Sub-slice 1: Write Regs (138 us)"),
    # Blocked on I2C hardware FIFO
    (4.396, 0.093, "I2C ISRs / Wait", "#e377c2", "Waiting for I2C Hardware FIFO\n(I2C_EXT0 ISR fires)"),
    # ACCEL_GYRO_DAQ resumes -> Sub-slice 2
    (4.489, 0.085, "ACCEL_GYRO_DAQ", "#1f77b4", "Sub-slice 2: Read FIFO (95 us)"),
    # Yield while waiting next transaction
    (4.574, 0.357, "System / Idle", "#7f7f7f", "Idle / Interrupts (I2C transfer)"),
    # ACCEL_GYRO_DAQ resumes -> Sub-slice 3
    (4.931, 0.109, "ACCEL_GYRO_DAQ", "#1f77b4", "Sub-slice 3: Calc & Push (137 us)\nSYSVIEW_END @ 5.040 ms")
]

# Track Y positions for contexts
y_map = {
    "ACCEL_GYRO_DAQ": 4,
    "BLOCKED (Lock Wait)": 3,
    "MAG_DAQ": 2,
    "MAG_DAQ Release": 2,
    "btController": 5,
    "ISRs (IPC & I2C)": 1,
    "Timer ISRs": 1,
    "I2C ISRs / Wait": 1,
    "System / Idle": 0
}

ax.set_ylim(-0.5, 6)
ax.set_xlim(3.2, 5.2)

# Draw blocks
for start, dur, context, color, text in segments:
    y = y_map.get(context, 0)
    rect = patches.Rectangle((start, y - 0.35), dur, 0.7, linewidth=1.5, edgecolor='black', facecolor=color, alpha=0.85)
    ax.add_patch(rect)

# Draw Pass #1 Marker overlay
marker_rect = patches.Rectangle((3.362, -0.2), 5.040 - 3.362, 5.8, linewidth=2, edgecolor='red', facecolor='none', linestyle='--')
ax.add_patch(marker_rect)
ax.text(3.38, 5.65, "Pass #1 Window: 1678 us (Worst-Case Execution Time / WCET)", color='red', fontweight='bold', fontsize=11)

# Annotations
ax.annotate("SYSVIEW_START(0)\n(t = 3.362 ms)", xy=(3.362, 4), xytext=(3.22, 4.8),
            arrowprops=dict(facecolor='black', shrink=0.05, width=1, headwidth=6), fontsize=9, fontweight='bold')

ax.annotate("vTaskPriorityInherit\n(i2c0_mutex contention)", xy=(3.403, 3), xytext=(3.42, 3.5),
            arrowprops=dict(facecolor='red', shrink=0.05, width=1, headwidth=6), fontsize=9, color='red', fontweight='bold')

ax.annotate("vTaskPriorityDisinherit\n(Mutex released)", xy=(4.209, 2), xytext=(4.05, 1.2),
            arrowprops=dict(facecolor='green', shrink=0.05, width=1, headwidth=6), fontsize=9, color='green', fontweight='bold')

ax.annotate("SYSVIEW_END(0)\n(t = 5.040 ms)", xy=(5.040, 4), xytext=(4.95, 4.8),
            arrowprops=dict(facecolor='black', shrink=0.05, width=1, headwidth=6), fontsize=9, fontweight='bold')

ax.set_yticks([0, 1, 2, 3, 4, 5])
ax.set_yticklabels(["System Idle", "Hardware ISRs", "MAG_DAQ (Prio 2)", "Mutex Lock Wait", "ACCEL_GYRO_DAQ (Prio 3)", "btController (Prio 23)"], fontsize=10, fontweight='bold')
ax.set_xlabel("Time (milliseconds)", fontsize=11, fontweight='bold')
ax.set_title("Slice-by-Slice Forensic Timeline: ACCEL_GYRO_DAQ Max Interruption Event (WCET = 1678 µs)", fontsize=13, fontweight='bold')
ax.grid(True, linestyle=':', alpha=0.6)

plt.tight_layout()
plt.savefig(output_path, dpi=200)
plt.close()
print(f"Plot saved to: {output_path}")
