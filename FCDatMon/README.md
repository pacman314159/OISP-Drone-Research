# **FCDatMon (Flight Controller Data Monitoring System)**

**FCDatMon** is a high-performance, real-time telemetry visualization engine designed for the OISP Drone Research project. Built to handle high-frequency data streams (e.g., 600Hz IMU updates) from an ESP32-S3 flight controller, it provides a comprehensive configuration interface coupled with hardware-accelerated 2D and 3D rendering. 

Unlike heavy IDE-based visualizers, FCDatMon prioritizes raw execution speed and deterministic thread management to ensure zero-lag data plotting during critical flight kinematics testing.

## **🚀 Key Features**

* **Hardware-Accelerated GUI:** Powered by DearPyGui (C++ Dear ImGui backend) for an ultra-fast, code-driven interface without the bloat of XML/UI designers. 
* **Dynamic 2D Plotting:** Real-time plotting with support for modular grid layouts (from 1x1 up to 3x3). Features hardware-accelerated panning, zooming, and bounding-box selections natively.
* **Smart Navigation Engine:** Automatically scrolls to chase live data streams while allowing seamless manual panning and zooming (Shift + Scroll) without interrupting background ingestion.
* **Multi-Protocol Support:** Architected to seamlessly swap between various hardware communication interfaces. (Currently, only the USB Serial module is implemented; BLE and LoRa are actively being developed to complete this vision):
  * **BLE (Bluetooth Low Energy):** Direct asynchronous connection to the ESP32-S3. *(WIP)*
  * **LoRa / USB:** Serial communication through a LoRa ground module or direct USB payload. *(USB Serial Complete)*
* **Robust Session Management:** Caches custom plot configurations (50-color palette picker, dynamic axis labels, custom limits) and saves exact telemetry dashboard layouts to `.json` files in your chosen directory for rapid re-deployment.
* **Raw Telemetry Logging:** Seamlessly capture incoming data from the transmitter and dump it directly into timestamped log files in any directory you choose (defaulting to `/captured_data/`).

## **🧠 System Architecture**

FCDatMon utilizes a strict **Producer-Consumer Architecture** to ensure that heavy data ingestion never blocks the visual rendering loop.

* **Producers:** Background threads handle I/O (BLE polling or Serial reading) and binary struct unpacking. 
* **Core Buffers (core/):** Thread-safe circular buffers (Ring Buffers) safely hold the telemetry streams, preventing RAM overflows during high-frequency data dumps. Sizes can be adjusted dynamically in the UI.
* **Consumers (gui/):** The main thread runs at a strict 60 FPS, slicing the latest data from the circular buffers and pushing it to the GPU for rendering.

### **Directory Structure**

```
FCDatMon/  
│  
├── main.py                   # Application entry point and GUI context initialization  
│  
├── saved_setups/             # User saved layout states (.json)  
├── captured_data/            # Captured raw telemetry data logs
│  
├── core/                     # Data management & Mathematics  
│   ├── data_manager.py       # Thread-safe circular buffers for high Hz telemetry  
│   ├── state_manager.py      # Session cache and layout JSON serialization  
│   ├── app_settings.py       # Persistent user preference manager
│   └── serial_reader.py      # Background worker for serial telemetry parsing
│  
└── gui/                      # Visual Rendering (Consumers)  
    ├── control_panel.py      # Control panel and configuration manager
    └── plot_2d_manager.py    # ImPlot 2D dynamic grid manager  
```

## **⚙️ Installation & Setup**

### **Prerequisites**

FCDatMon is built entirely in Python but relies on specific C++ wrapped libraries for performance. Ensure you have Python 3.10+ installed.

### **Dependencies**

Install the required packages using pip: 
```bash
pip install dearpygui pyserial
```

## **🏁 Usage**

To launch the monitoring system, run the main entry point from your terminal: 
```bash
python main.py
```

1. **Configure Hardware:** In the Control Panel, select your protocol (BLE, LoRa, USB) and specify the target MAC address or COM port. 
2. **Set Layout:** Choose your desired grid size (e.g., 2x3). The UI will dynamically generate plot configurations. 
3. **Connect:** Click `[ CONNECT ]` to spawn the background thread and begin data ingestion. 
4. **Save/Load Layouts:** Enter a setup name and click `[ SAVE ]` to preserve your layout and axis configurations for future flights.
5. **Capture Data:** Check the "Save Captured Data to Disk" box and select your desired folder to automatically log incoming telemetry into timestamped files.

## **📦 Building an Executable**

FCDatMon can be easily compiled into a standalone app and packaged into a professional Setup Wizard so it can be deployed to other machines without installing Python.

1. **Install PyInstaller:**
   ```bash
   pip install pyinstaller
   ```
2. **Compile Application:**
   Run the following command in the root project directory:
   ```bash
   pyinstaller FCDatMon.spec --clean --noconfirm
   ```
3. **Build Installer Wizard:**
   Open the `installation_compiler/script.iss` file using **Inno Setup** and hit Compile. This packs the generated `dist/FCDatMon/` bundle into a professional `setup.exe` installer, complete with Start Menu shortcuts and correct directory structures.

## **🚧 Current Status**

* **Phase 1 (Complete):** Core GUI framework built using DearPyGui. Session management (saving/loading layouts dynamically) and telemetry dumping systems are fully functional. UI is highly polished with robust numeric input validation, an interactive 50-color distinct palette picker, custom window anchoring, and dynamic layout handling.
* **Phase 2 (Complete):** Implementation of the background telemetry producers (Serial) and thread-safe dynamic ring buffers. Integrated Smart Navigation Engine with complete interaction overrides.
* **Phase 3 (WIP):** Full implementation of BLE asynchronous polling and binary payload struct parsing for the ESP32-S3.
