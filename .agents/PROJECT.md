# Drone Research 2026 - Modular Architecture

Overall mechatronics repository organized into decoupled domain modules. Each module maintains its own scoped `.agents/` configuration, skills, and feature tracking:

- **`firmware/FC_v01/`**: ESP32-S3 FreeRTOS flight controller firmware (5-layer RTOS, IMU DAQ, attitude estimation, cascade PID, BLE telemetry). See `firmware/FC_v01/.agents/PROJECT.md`.
- **`analyzers/`**: Python DSP pipelines (FFT, filter characterization) and FreeRTOS SystemView timing trace scripts.
- **`system_architecture/`**: System architecture specification documents (`System Architecture v01.pdf`).
- **`research_docs/` & `datasheets/`**: Academic papers, mathematical derivations, and hardware component datasheets.
