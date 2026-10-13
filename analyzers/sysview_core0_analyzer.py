#!/usr/bin/env python3
"""
SEGGER SystemView CSV Trace Analyzer for Flight Controller (FC_v01)
Parses exported SystemView event CSV logs to extract timing statistics,
jitter, execution budgets (WCET), ISR overhead, and RTOS resource contention.
"""

import os
import sys
import re
import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Ensure safe UTF-8 output on Windows consoles
if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')


def parse_duration_us(detail_str):
    if not isinstance(detail_str, str):
        return None
    m = re.search(r'(?:Runs for|Ran for|Idle for)\s+([\d.]+)\s+(us|ms|s)', detail_str, re.IGNORECASE)
    if m:
        val = float(m.group(1))
        unit = m.group(2).lower()
        if unit == 'us':
            return val
        elif unit == 'ms':
            return val * 1000.0
        elif unit == 's':
            return val * 1000000.0
    return None


def analyze_sysview_csv(csv_path, output_dir=None, show_plot=False):
    if not os.path.exists(csv_path):
        print(f"Error: File not found: {csv_path}")
        return

    if output_dir is None:
        output_dir = os.path.dirname(csv_path)

    print(f"\n=======================================================")
    print(f"   SEGGER SystemView Trace Analysis: {os.path.basename(csv_path)}")
    print(f"=======================================================\n")

    # Read CSV
    df = pd.read_csv(csv_path)
    total_events = len(df)

    # Session metadata
    start_ts_us = df['timestampint'].iloc[0]
    end_ts_us = df['timestampint'].iloc[-1]
    total_duration_us = end_ts_us - start_ts_us
    total_duration_s = total_duration_us / 1e6

    print(f"Trace Duration     : {total_duration_s:.4f} s ({total_duration_us / 1000.0:.2f} ms)")
    print(f"Total Events Logged: {total_events:,}")

    # Extract Task & Stack Metadata
    tasks_info = {}
    isrs_info = {}
    priority_inheritance_events = []

    # Map ISR descriptions from System Description events
    sys_desc_df = df[df['event'] == 'System Description']
    for _, row in sys_desc_df.iterrows():
        detail = str(row['detail'])
        m_irq = re.match(r'I#(\d+)=([a-zA-Z0-9_]+)', detail)
        if m_irq:
            isrs_info[int(m_irq.group(1))] = m_irq.group(2)

    # Task info & Stack Info
    for _, row in df.iterrows():
        event = row['event']
        detail = str(row['detail'])
        if event == 'Task Info':
            m = re.search(r'([a-zA-Z0-9_]+)\s+\((0x[0-9a-fA-F]+)\):\s+Priority=(\d+)', detail)
            if m:
                name, addr, prio = m.group(1), m.group(2), int(m.group(3))
                if name not in tasks_info:
                    tasks_info[name] = {'name': name, 'addr': addr, 'prio': prio, 'stack_free': None, 'stack_addr': None}
                else:
                    tasks_info[name]['prio'] = prio
        elif event == 'Stack Info':
            m = re.search(r'([a-zA-Z0-9_]+)\s+\((0x[0-9a-fA-F]+)\):\s+(\d+)\s+@\s+(0x[0-9a-fA-F]+)', detail)
            if m:
                name, addr, stack_free, stack_addr = m.group(1), m.group(2), int(m.group(3)), m.group(4)
                if name not in tasks_info:
                    tasks_info[name] = {'name': name, 'addr': addr, 'prio': None, 'stack_free': stack_free, 'stack_addr': stack_addr}
                else:
                    tasks_info[name]['stack_free'] = stack_free
                    tasks_info[name]['stack_addr'] = stack_addr
        elif event in ('vTaskPriorityInherit', 'vTaskPriorityDisinherit'):
            priority_inheritance_events.append({
                'timestamp_ms': row['timestampint'] / 1000.0,
                'task': row['context'],
                'event': event,
                'detail': detail
            })

    # Task Execution Analysis
    task_run_df = df[df['event'] == 'Task Run'].copy()
    task_run_df['duration_us'] = task_run_df['detail'].apply(parse_duration_us)

    # Map Tasks to their SystemView Marker IDs
    # (From config.h: TASK_ACCEL_GYRO_DAQ_ID=0 -> Marker 1, TASK_MAG_DAQ_ID=3 -> Marker 4, TASK_BLE_VEC3_TRANS_ID=4 -> Marker 5)
    task_marker_map = {
        'ACCEL_GYRO_DAQ': '0x00000001',
        'MAG_DAQ': '0x00000004',
        'BLE_VEC3_TRANS_': '0x00000005',
    }

    # Extract Full Loop Iterations from Start Markers
    loop_stats = []
    task_intervals = {}

    for task_name in df['context'].unique():
        if task_name in ('Idle', 'SysTick', 'Scheduler', '*** OVERFLOW ***'):
            continue

        marker_id = task_marker_map.get(task_name)
        start_marker_name = f"Start Marker {marker_id}" if marker_id else None
        stop_marker_name = f"Stop Marker {marker_id}" if marker_id else None

        marker_starts = df[(df['context'] == task_name) & (df['event'] == start_marker_name)] if start_marker_name else pd.DataFrame()
        marker_stops = df[(df['context'] == task_name) & (df['event'] == stop_marker_name)] if stop_marker_name else pd.DataFrame()

        # If markers exist, use them for true loop period & end-to-end iteration duration
        if len(marker_starts) > 1:
            loop_timestamps_us = marker_starts['timestampint'].values
            loop_periods_us = np.diff(loop_timestamps_us)
            
            # Filter out OpenOCD buffer overflow gaps
            max_valid_gap = 10000.0 if task_name in ('ACCEL_GYRO_DAQ', 'BLE_VEC3_TRANS_') else 50000.0
            valid_p_mask = loop_periods_us < max_valid_gap
            valid_periods = loop_periods_us[valid_p_mask] if np.any(valid_p_mask) else loop_periods_us
            
            task_intervals[task_name] = valid_periods
            loop_count = len(marker_starts)
            avg_loop_period_us = np.mean(valid_periods) if len(valid_periods) > 0 else 0
            jitter_loop_us = np.std(valid_periods) if len(valid_periods) > 0 else 0
            min_loop_period_us = np.min(valid_periods) if len(valid_periods) > 0 else 0
            max_loop_period_us = np.max(valid_periods) if len(valid_periods) > 0 else 0
            meas_freq_hz = (1e6 / avg_loop_period_us) if avg_loop_period_us > 0 else 0

            # End-to-end loop iteration duration from Stop Marker detail (filtered)
            max_valid_rt = 5000.0 if task_name == 'ACCEL_GYRO_DAQ' else (50000.0 if task_name == 'MAG_DAQ' else 1000.0)
            iteration_durs = marker_stops['detail'].apply(parse_duration_us).dropna().values
            valid_iter_durs = iteration_durs[(iteration_durs > 0) & (iteration_durs < max_valid_rt)] if len(iteration_durs) > 0 else iteration_durs
            avg_iter_us = np.mean(valid_iter_durs) if len(valid_iter_durs) > 0 else 0
            max_iter_us = np.max(valid_iter_durs) if len(valid_iter_durs) > 0 else 0
            min_iter_us = np.min(valid_iter_durs) if len(valid_iter_durs) > 0 else 0
        else:
            # Fallback: cluster Task Run events separated by sleep delays (> 500 us)
            t_runs = df[(df['context'] == task_name) & (df['event'] == 'Task Run')]
            if len(t_runs) == 0:
                continue
            ts = t_runs['timestampint'].values
            diffs = np.diff(ts)
            loop_starts = [ts[0]] + [ts[i+1] for i, d in enumerate(diffs) if d > 500]
            loop_count = len(loop_starts)
            if len(loop_starts) > 1:
                loop_periods_us = np.diff(loop_starts)
                task_intervals[task_name] = loop_periods_us
                avg_loop_period_us = np.mean(loop_periods_us)
                jitter_loop_us = np.std(loop_periods_us)
                meas_freq_hz = (1e6 / avg_loop_period_us) if avg_loop_period_us > 0 else 0
            else:
                avg_loop_period_us = 0
                jitter_loop_us = 0
                meas_freq_hz = 0

            durs = t_runs['detail'].apply(parse_duration_us).dropna().values
            avg_iter_us = np.mean(durs) if len(durs) > 0 else 0
            max_iter_us = np.max(durs) if len(durs) > 0 else 0
            min_iter_us = np.min(durs) if len(durs) > 0 else 0

        # Total CPU run time
        t_all_runs = df[(df['context'] == task_name) & (df['event'] == 'Task Run')]
        durs_all = t_all_runs['detail'].apply(parse_duration_us).dropna().values
        total_cpu_us = np.sum(durs_all) if len(durs_all) > 0 else 0
        cpu_load_pct = (total_cpu_us / total_duration_us) * 100.0

        prio = tasks_info.get(task_name, {}).get('prio', '-')
        free_stack = tasks_info.get(task_name, {}).get('stack_free', '-')

        loop_stats.append({
            'Task': task_name,
            'Prio': prio,
            'Loop_Count': loop_count,
            'Meas_Freq_Hz': meas_freq_hz,
            'Avg_Period_us': avg_loop_period_us,
            'Jitter_us': jitter_loop_us,
            'Avg_Iteration_us': avg_iter_us,
            'WCET_Iteration_us': max_iter_us,
            'CPU_Load_Pct': cpu_load_pct,
            'Free_Stack_B': free_stack
        })

    loop_stats.sort(key=lambda x: x['CPU_Load_Pct'], reverse=True)

    # ISR Analysis
    isr_enter_df = df[df['event'] == 'ISR Enter'].copy()
    isr_enter_df['duration_us'] = isr_enter_df['detail'].apply(parse_duration_us)

    isr_stats = []
    for isr_name in isr_enter_df['context'].unique():
        i_df = isr_enter_df[isr_enter_df['context'] == isr_name]
        durs = i_df['duration_us'].dropna().values
        invocations = len(i_df)
        total_isr_us = np.sum(durs) if len(durs) > 0 else 0
        isr_load_pct = (total_isr_us / total_duration_us) * 100.0

        min_d = np.min(durs) if len(durs) > 0 else 0
        avg_d = np.mean(durs) if len(durs) > 0 else 0
        max_d = np.max(durs) if len(durs) > 0 else 0

        isr_stats.append({
            'ISR': isr_name,
            'Invocations': invocations,
            'Min_us': min_d,
            'Avg_us': avg_d,
            'Max_us': max_d,
            'CPU_Load_Pct': isr_load_pct
        })

    isr_stats.sort(key=lambda x: x['CPU_Load_Pct'], reverse=True)

    # User Markers Analysis (SYSVIEW_START / SYSVIEW_END)
    markers_df = df[df['event'].str.contains('Stop Marker', na=False)].copy()
    markers_df['duration_us'] = markers_df['detail'].apply(parse_duration_us)
    marker_stats = []
    for marker_name in markers_df['event'].unique():
        m_df = markers_df[markers_df['event'] == marker_name]
        durs = m_df['duration_us'].dropna().values
        if len(durs) > 0:
            marker_stats.append({
                'Marker': marker_name,
                'Task': m_df['context'].iloc[0],
                'Passes': len(durs),
                'Min_us': np.min(durs),
                'Avg_us': np.mean(durs),
                'Max_us': np.max(durs)
            })

    # Print Formatted Report to Console
    print("\n-----------------------------------------------------------------------------------------------------------------")
    # Print Formatted Report to Console
    print("\n-----------------------------------------------------------------------------------------------------------------------------")
    print(" 1. TASK FULL LOOP ITERATION TIMING & EXECUTION BUDGET")
    print("    (Measured across full iterations from loop wakeup / SYSVIEW_START to SYSVIEW_END)")
    print("-----------------------------------------------------------------------------------------------------------------------------")
    header = f"{'Task Name':<18} | {'Prio':<4} | {'Loops':<5} | {'Freq (Hz)':<10} | {'Period (us)':<12} | {'Jitter (us)':<12} | {'Avg Run (us)':<12} | {'WCET (us)':<10} | {'CPU%':<6} | {'StackFree':<9}"
    print(header)
    print("-" * len(header))
    for t in loop_stats:
        print(f"{t['Task']:<18} | {str(t['Prio']):<4} | {t['Loop_Count']:<5} | {t['Meas_Freq_Hz']:<10.1f} | {t['Avg_Period_us']:<12.1f} | {t['Jitter_us']:<12.1f} | {t['Avg_Iteration_us']:<12.1f} | {t['WCET_Iteration_us']:<10.1f} | {t['CPU_Load_Pct']:<6.2f} | {str(t['Free_Stack_B']):<9}")

    print("\n-----------------------------------------------------------------------------------------------------------------------------")
    print(" 2. HARDWARE INTERRUPTS (ISRs)")
    print("-----------------------------------------------------------------------------------------------------------------------------")
    header_isr = f"{'ISR Name':<20} | {'Count':<7} | {'Min (us)':<10} | {'Avg (us)':<10} | {'Max (us)':<10} | {'CPU Load %':<10}"
    print(header_isr)
    print("-" * len(header_isr))
    for i in isr_stats:
        print(f"{i['ISR']:<20} | {i['Invocations']:<7} | {i['Min_us']:<10.1f} | {i['Avg_us']:<10.1f} | {i['Max_us']:<10.1f} | {i['CPU_Load_Pct']:<10.2f}")

    if marker_stats:
        print("\n-----------------------------------------------------------------------------------------------------------------------------")
        print(" 3. INSTRUMENTED CODE BLOCKS (SYSVIEW MARKERS)")
        print("-----------------------------------------------------------------------------------------------------------------------------")
        header_m = f"{'Marker':<24} | {'Context':<18} | {'Passes':<6} | {'Min (us)':<10} | {'Avg (us)':<10} | {'Max (us)':<10}"
        print(header_m)
        print("-" * len(header_m))
        for m in marker_stats:
            print(f"{m['Marker']:<24} | {m['Task']:<18} | {m['Passes']:<6} | {m['Min_us']:<10.1f} | {m['Avg_us']:<10.1f} | {m['Max_us']:<10.1f}")

    if priority_inheritance_events:
        print("\n-----------------------------------------------------------------------------------------------------------------------------")
        print(f" 4. RESOURCE LOCK CONTENTION & PRIORITY INHERITANCE ({len(priority_inheritance_events)} events)")
        print("-----------------------------------------------------------------------------------------------------------------------------")
        for ev in priority_inheritance_events[:10]:
            print(f"  [{ev['timestamp_ms']:9.3f} ms] Task: {ev['task']:<16} Event: {ev['event']:<24} Detail: {ev['detail']}")
        if len(priority_inheritance_events) > 10:
            print(f"  ... and {len(priority_inheritance_events) - 10} more events.")

    # Flight Controller Real-Time Health Diagnostics
    print("\n-----------------------------------------------------------------------------------------------------------------------------")
    print(" 5. FLIGHT CONTROLLER REAL-TIME HEALTH CHECK")
    print("-----------------------------------------------------------------------------------------------------------------------------")
    
    imu_task = next((t for t in loop_stats if 'ACCEL_GYRO' in t['Task']), None)
    if imu_task:
        target_period_us = 2000.0  # 500 Hz
        headroom_pct = (1.0 - (imu_task['WCET_Iteration_us'] / target_period_us)) * 100.0 if target_period_us > 0 else 0
        print(f"  [+] 500 Hz IMU Loop Execution Budget:")
        print(f"      - Configured Target      : {target_period_us:.1f} us (500.0 Hz)")
        print(f"      - Loop Measured Rate     : {imu_task['Meas_Freq_Hz']:.1f} Hz (Avg Loop Period: {imu_task['Avg_Period_us']:.1f} us)")
        print(f"      - Timing Jitter (StdDev) : {imu_task['Jitter_us']:.1f} us")
        print(f"      - Full Loop Iteration    : Avg: {imu_task['Avg_Iteration_us']:.1f} us | Worst-Case (WCET): {imu_task['WCET_Iteration_us']:.1f} us")
        print(f"      - Flight Loop Headroom   : {headroom_pct:.2f}% (Safety margin for PID & Attitude Estimation)")
        if headroom_pct > 70.0:
            print(f"      -> STATUS: EXCELLENT. Massive computational headroom available.")
        elif headroom_pct > 30.0:
            print(f"      -> STATUS: HEALTHY. Sufficient margin.")
        else:
            print(f"      -> WARNING: Low headroom. Consider optimizing I2C transactions or math loops.")

    # Save summary report to markdown
    report_path = os.path.join(output_dir, "sysview_analysis_report.md")
    with open(report_path, "w", encoding="utf-8") as f:
        f.write(f"# SEGGER SystemView Session Analysis Report\n\n")
        f.write(f"- **Source File**: `{os.path.basename(csv_path)}`\n")
        f.write(f"- **Trace Duration**: {total_duration_s:.4f} s ({total_duration_us / 1000.0:.2f} ms)\n")
        f.write(f"- **Total Events**: {total_events:,}\n\n")
        
        f.write("## 1. Task Full Loop Timing Statistics\n\n")
        f.write("| Task | Priority | Loops | Freq (Hz) | Period (µs) | Jitter (µs) | Avg Iteration (µs) | WCET (µs) | CPU Load (%) | Free Stack (Bytes) |\n")
        f.write("| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |\n")
        for t in loop_stats:
            f.write(f"| {t['Task']} | {t['Prio']} | {t['Loop_Count']} | {t['Meas_Freq_Hz']:.1f} | {t['Avg_Period_us']:.1f} | {t['Jitter_us']:.1f} | {t['Avg_Iteration_us']:.1f} | {t['WCET_Iteration_us']:.1f} | {t['CPU_Load_Pct']:.2f}% | {t['Free_Stack_B']} |\n")

        f.write("\n## 2. Hardware Interrupts (ISRs)\n\n")
        f.write("| ISR Name | Invocations | Min (µs) | Avg (µs) | Max (µs) | CPU Load (%) |\n")
        f.write("| :--- | :--- | :--- | :--- | :--- | :--- |\n")
        for i in isr_stats:
            f.write(f"| {i['ISR']} | {i['Invocations']} | {i['Min_us']:.1f} | {i['Avg_us']:.1f} | {i['Max_us']:.1f} | {i['CPU_Load_Pct']:.2f}% |\n")

        if marker_stats:
            f.write("\n## 3. Custom Instrumentation Markers\n\n")
            f.write("| Marker | Context | Passes | Min (µs) | Avg (µs) | Max (µs) |\n")
            f.write("| :--- | :--- | :--- | :--- | :--- | :--- |\n")
            for m in marker_stats:
                f.write(f"| {m['Marker']} | {m['Task']} | {m['Passes']} | {m['Min_us']:.1f} | {m['Avg_us']:.1f} | {m['Max_us']:.1f} |\n")

    print(f"\n[+] Saved detailed Markdown report: {report_path}")

    # Generate Visualization Figure
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle(f"SEGGER SystemView Real-Time Analysis ({os.path.basename(csv_path)})", fontsize=14, fontweight='bold')

    # Plot 1: WCET vs Avg Run Time
    ax1 = axes[0, 0]
    task_names = [t['Task'] for t in loop_stats if t['Task'] != 'Idle'][:6]
    avg_runs = [t['Avg_Iteration_us'] for t in loop_stats if t['Task'] in task_names]
    wcet_runs = [t['WCET_Iteration_us'] for t in loop_stats if t['Task'] in task_names]

    x = np.arange(len(task_names))
    width = 0.35
    ax1.bar(x - width/2, avg_runs, width, label='Avg Iteration (µs)', color='#2b5c8f')
    ax1.bar(x + width/2, wcet_runs, width, label='WCET (µs)', color='#e05d44')
    ax1.set_xticks(x)
    ax1.set_xticklabels(task_names, rotation=25, ha='right')
    ax1.set_ylabel('Execution Time (µs)')
    ax1.set_title('Task Execution: Avg Iteration vs. Worst-Case (WCET)')
    ax1.legend()
    ax1.grid(axis='y', linestyle='--', alpha=0.6)

    # Plot 2: CPU Load Breakdown
    ax2 = axes[0, 1]
    top_tasks = [t for t in loop_stats[:5]]
    labels = [t['Task'] for t in top_tasks]
    loads = [t['CPU_Load_Pct'] for t in top_tasks]
    other_load = max(0, 100.0 - sum(loads))
    if other_load > 0:
        labels.append('Other / System')
        loads.append(other_load)
    ax2.pie(loads, labels=labels, autopct='%1.1f%%', startangle=140,
            colors=plt.cm.Paired(np.linspace(0, 1, len(labels))))
    ax2.set_title('CPU Load Breakdown (%)')

    # Plot 3: 500 Hz IMU Loop Jitter (Interval between wakeups)
    ax3 = axes[1, 0]
    for candidate in ['ACCEL_GYRO_DAQ', 'MAG_DAQ']:
        if candidate in task_intervals:
            intervals_us = task_intervals[candidate]
            ax3.plot(intervals_us, label=f"{candidate} (Period µs)", alpha=0.8)
    ax3.set_ylabel('Period (µs)')
    ax3.set_xlabel('Iteration Sequence')
    ax3.set_title('Task Period Jitter Over Time')
    ax3.legend()
    ax3.grid(True, linestyle='--', alpha=0.6)

    # Plot 4: Remaining Free Stack Margin
    ax4 = axes[1, 1]
    stack_tasks = [t['Task'] for t in loop_stats if isinstance(t['Free_Stack_B'], (int, float))]
    stack_vals = [t['Free_Stack_B'] for t in loop_stats if isinstance(t['Free_Stack_B'], (int, float))]
    if stack_tasks:
        colors = ['#2ca02c' if s > 1024 else '#d62728' for s in stack_vals]
        ax4.barh(stack_tasks, stack_vals, color=colors)
        ax4.set_xlabel('Free Stack Remaining (Bytes)')
        ax4.set_title('Task Stack High-Water Mark (Safety Margin)')
        ax4.axvline(1024, color='orange', linestyle='--', label='1 KB Warning')
        ax4.legend()
        ax4.grid(axis='x', linestyle='--', alpha=0.6)


    plt.tight_layout()
    plot_path = os.path.join(output_dir, "sysview_analysis_plot.png")
    plt.savefig(plot_path, dpi=200)
    print(f"[+] Saved visualization plot: {plot_path}")

    if show_plot:
        plt.show()
    plt.close()


if __name__ == '__main__':
    default_csv = r"D:\OneDrive_MSFT\Work\Drone_Research\analyzers\SysView data\core0.csv"
    target_path = sys.argv[1] if len(sys.argv) > 1 else default_csv
    analyze_sysview_csv(target_path)
