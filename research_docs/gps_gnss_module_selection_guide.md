# Multirotor GNSS/GPS Module Selection, Antenna Physics & Drone Integration Guide

> **Author**: Antigravity Autonomous Engineering & Drone Research  
> **Project Target**: 450mm (F450-class) and Sub-450mm Multirotors  
> **Firmware Target**: ESP32-S3 Dual-Core FreeRTOS Flight Controller (`FC_v01`)  
> **Date**: November 2026  

---

## Table of Contents
1. [Executive Summary & Technical Architecture](#1-executive-summary--technical-architecture)
2. [GNSS Silicon & Module Generations (The "Brains")](#2-gnss-silicon--module-generations-the-brains)
   - [u-blox NEO-6M / GY-NEO6MV2 (Legacy / Budget)](#21-u-blox-neo-6m--gy-neo6mv2-legacy--budget)
   - [Beitian BN-880 (The F450 Budget Workhorse)](#22-beitian-bn-880-the-f450-budget-workhorse)
   - [Matek M10-5883 / M10Q-5883 (Modern High-Performance Standard)](#23-matek-m10-5883--m10q-5883-modern-high-performance-standard)
   - [Holybro Micro M10 & M10 GPS Puck (Professional Grade)](#24-holybro-micro-m10--m10-gps-puck-professional-grade)
   - [Matek M10-L4-3100 (DroneCAN & UART Hybrid)](#25-matek-m10-l4-3100-dronecan--uart-hybrid)
   - [Holybro H-RTK F9P Series (Centimeter Precision RTK)](#26-holybro-h-rtk-f9p-series-centimeter-precision-rtk)
3. [Price vs. Specification Comparison Matrix](#3-price-vs-specification-comparison-matrix)
4. [Antenna Physics: Flat Ceramic Patch vs. Helical "Tower"](#4-antenna-physics-flat-ceramic-patch-vs-helical-tower)
   - [Flat Ceramic Patch Antennas (18x18mm vs. 25x25mm)](#41-flat-ceramic-patch-antennas-18x18mm-vs-25x25mm)
   - [Quadrifilar Helix Antennas (QHA / Helical "Tower")](#42-quadrifilar-helix-antennas-qha--helical-tower)
5. [The "Tower" (Elevated Standoff Mast): Why Multirotors Need It](#5-the-tower-elevated-standoff-mast-why-multirotors-need-it)
   - [Reason 1: Compass Magnetic Isolation (Ampère's Law & Toilet-Bowling)](#51-reason-1-compass-magnetic-isolation-ampères-law--toilet-bowling)
   - [Reason 2: RF EMI Near-Field Isolation](#52-reason-2-rf-emi-near-field-isolation)
   - [Reason 3: Horizon Line-of-Sight & LiPo Blocking](#53-reason-3-horizon-line-of-sight--lipo-blocking)
6. [Recommendations: 450mm vs. Sub-250mm Quads](#6-recommendations-450mm-vs-sub-250mm-quads)
7. [Hardware & Firmware Integration for ESP32-S3 (`FC_v01`)](#7-hardware--firmware-integration-for-esp32-s3-fc_v01)

---

## 1. Executive Summary & Technical Architecture

With vertical altitude hold successfully achieved via barometer and vertical accelerometer fusion, expanding to **3D horizontal position hold (Loiter / PosHold)**, **Return-to-Home (RTH)**, and **waypoint navigation** requires adding a Global Navigation Satellite System (GNSS) receiver.

A multirotor positioning system relies on two critical sensing elements:
1. **The GNSS Receiver**: Resolves latitude, longitude, ellipsoidal altitude, ground speed, and heading course over ground via RF time-of-flight from satellite constellations.
2. **The 3-Axis Magnetometer (Digital Compass)**: Resolves the vehicle's true geomagnetic heading (yaw) relative to magnetic north. **A multirotor cannot calculate yaw from GPS alone while hovering in place** (velocity is zero). Without an accurate compass heading, horizontal position hold will diverge into a destructive circling oscillation known as **"toilet-bowling"**.

For this reason, drone GPS modules almost universally combine a **GNSS receiver chip + a 3-axis magnetometer + a ceramic patch antenna** on a single board.

---

## 2. GNSS Silicon & Module Generations (The "Brains")

### 2.1 u-blox NEO-6M / GY-NEO6MV2 (Legacy / Budget)

![GY-NEO6MV2 NEO-6M GPS Module](https://38-3d.co.uk/cdn/shop/files/GY-NEO6MV2-NEO-6M-GPS-Module.webp?v=1778402597)
*Figure 1: GY-NEO6MV2 breakout board with external 25x25mm patch antenna on u.FL cable.*

| Parameter | Specification |
| :--- | :--- |
| **Chipset** | u-blox NEO-6M (ROM-based 6th Generation) |
| **Constellations** | Single constellation: **GPS L1 C/A only** (1575.42 MHz) |
| **Refresh Rate** | 1 Hz default (5 Hz maximum, unstable at 5 Hz with NMEA) |
| **Concurrent Satellites** | 6 – 10 satellites typically |
| **Compass** | **None** (Requires separate external compass board) |
| **Time to First Fix (Cold)** | 35 – 50 seconds |
| **Interface** | UART (Default: 9600 baud, 3.3V logic) |
| **Current Draw** | ~67 mA @ 3.3V |
| **Price** | **\$6 – \$12 USD** |

* **Pros**: Inexpensive, universally documented for Arduino beginner tutorials.
* **Cons**: Single constellation means high dilution of precision (HDOP > 1.5). Slow 1–5 Hz rate induces phase lag in position control loops. Market is heavily flooded with counterfeit/re-marked chips. No onboard compass.
* **Verdict**: **Avoid for modern drone builds.**

---

### 2.2 Beitian BN-880 (The F450 Budget Workhorse)

![Beitian BN-880 GPS & Compass Module](https://www.flyingtech.co.uk/wp-content/uploads/2020/03/BN-880-GPS.jpg)
*Figure 2: Beitian BN-880 module showing integrated 25x25mm ceramic patch antenna and 6-pin wiring harness.*

![Beitian BN-880 Pinout & Dimensions](https://manuals.plus/m/00c71380bd8d0a583d124128c3abdf076febd77ac5c2965e40c774d2d4bc7e2f.png)
*Figure 3: BN-880 pinout: Pin 1=SDA, Pin 2=GND, Pin 3=TX, Pin 4=RX, Pin 5=VCC, Pin 6=SCL.*

| Parameter | Specification |
| :--- | :--- |
| **Chipset** | u-blox NEO-M8N compatible (M8030-KT) + Flash memory |
| **Constellations** | Dual concurrent: **GPS + GLONASS** or **GPS + BeiDou** + SBAS/QZSS |
| **Refresh Rate** | Up to 10 Hz (Dual GNSS) |
| **Concurrent Satellites** | 14 – 20 satellites |
| **Compass** | **Integrated QMC5883L / HMC5883L** 3-Axis Magnetometer (I2C) |
| **Antenna** | $25 \times 25 \times 4\text{ mm}$ Ceramic Patch Antenna |
| **Interface** | UART (115200 baud recommended) + I2C (Compass) |
| **Dimensions / Weight** | $28 \times 28 \times 10\text{ mm}$ / $10\text{ g}$ bare (28g with cable/mount) |
| **Price** | **\$16 – \$22 USD** |

* **Pros**: Proven record in DIY F450 and APM/Pixhawk drones for nearly a decade. Flash memory allows saving configuration. Large 25x25mm patch yields solid SNR. Built-in I2C compass saves wiring space.
* **Cons**: Uses older M8 silicon architecture. Power consumption is higher than M10 (~55 mA). Cannot track all 4 major constellations concurrently.
* **Verdict**: **Best entry-level budget choice for F450 drones.**

---

### 2.3 Matek M10-5883 / M10Q-5883 (Modern High-Performance Standard)

![Matek M10-5883 Official Photo](https://www.mateksys.com/wp-content/uploads/2022/06/M10-5883_1.jpg)
*Figure 4: Matek M10-5883 front and back sides showing u-blox MAX-M10S, QMC5883L, and LDO filtering.*

![Matek M10Q-5883](https://www.mateksys.com/wp-content/uploads/2023/03/M10Q-5883_1.jpg)
*Figure 5: Matek M10Q-5883 variant with compact 18x18mm ceramic patch antenna.*

| Parameter | Specification |
| :--- | :--- |
| **Chipset** | **u-blox 10th Gen (MAX-M10S)** + Flash Memory |
| **Constellations** | **4 Concurrent GNSS**: GPS (L1C/A), GLONASS (L1OF), Galileo (E1-B/C), BeiDou (B1I) + SBAS/QZSS |
| **Refresh Rate** | 10 Hz default, up to **18 Hz** (4 GNSS concurrent) |
| **Concurrent Satellites** | **28 – 35+ satellites simultaneously** |
| **Compass** | **Integrated QMC5883L** (I2C) |
| **Antenna** | $18 \times 18 \times 4\text{ mm}$ patch (M10Q) or $25 \times 25\text{ mm}$ (M10-5883) |
| **Power Consumption** | Ultra-low: **~19 mA @ 3.3V** (nearly 3x more efficient than M8N) |
| **Dimensions / Weight** | $20 \times 20 \times 6.5\text{ mm}$ / **4.5 grams** |
| **Price** | **\$26 – \$32 USD** |

* **Pros**: Unmatched satellite count (locks onto 30+ birds within 15 seconds). 18 Hz update rate allows razor-sharp position and velocity estimates. Extreme power efficiency. Low RF noise floor.
* **Cons**: Small 18x18mm patch on M10Q has slightly less antenna gain than full-size 25x25mm pucks under dense tree foliage.
* **Verdict**: **The definitive recommendation for modern quadcopter flight controllers.**

---

### 2.4 Holybro Micro M10 & M10 GPS Puck (Professional Grade)

![Holybro Micro M10 GPS](https://holybro.com/cdn/shop/files/12043_1_720x.jpg?v=1775198312)
*Figure 6: Holybro Micro M10 module with industrial-grade IST8310 compass and metal RF shield.*

![Holybro M10 GPS Module Puck](https://holybro.com/cdn/shop/files/M10GPS-1_1080x.jpg?v=1776755127)
*Figure 7: Holybro enclosed M10 GPS puck with built-in status RGB LED and mast collar.*

| Parameter | Specification |
| :--- | :--- |
| **Chipset** | u-blox MAX-M10S + Internal Flash |
| **Constellations** | 4 Concurrent GNSS (GPS + GLONASS + Galileo + BeiDou) |
| **Refresh Rate** | 10 Hz – 18 Hz |
| **Compass** | **iSentek IST8310** (High-precision industrial magnetometer, lower thermal drift than QMC5883L) |
| **Antenna** | High-gain $25 \times 25\text{ mm}$ tuned ceramic patch |
| **Enclosure** | CNC / Polycarbonate shielded puck enclosure with integrated RGB status LED and safety switch |
| **Weight** | Micro: $13\text{ g}$ / Enclosed Puck: $32\text{ g}$ |
| **Price** | **\$35 – \$48 USD** |

* **Pros**: Industrial build quality. IST8310 compass is noticeably more resilient against thermal drift and magnetic noise. Puck version is fully shielded from EMI and ready to clamp straight onto a carbon mast.
* **Cons**: Higher cost, heavier puck form factor.
* **Verdict**: **The top tier pick for professional and enterprise 450mm platforms.**

---

### 2.5 Matek M10-L4-3100 (DroneCAN & UART Hybrid)

![Matek M10-L4-3100 Module](https://www.mateksys.com/wp-content/uploads/2022/06/M10-L4-3100_1.jpg)
*Figure 8: Matek M10-L4-3100 with onboard STM32L431 companion MCU, CAN transceiver, and LIS3MDL compass.*

| Parameter | Specification |
| :--- | :--- |
| **Chipset** | u-blox MAX-M10S + **STM32L431 Companion MCU** |
| **Bus Protocols** | **DroneCAN (CAN Bus)** + Dual UART (MSP / UBX) |
| **Integrated Sensors** | Compass (ST LIS3MDL) + High-Precision Barometer (SPL06-001) |
| **Antenna** | $25 \times 25\text{ mm}$ ceramic patch |
| **Price** | **\$42 – \$48 USD** |

* **Pros**: Eliminates long UART/I2C serial runs by transmitting CAN differential packets immune to electrical noise. Adds a redundant secondary barometer.
* **Cons**: Requires CAN transceiver and DroneCAN stack in firmware (more complex than simple UART).
* **Verdict**: **Overkill for basic serial builds, but optimal for high-interference industrial frames.**

---

### 2.6 Holybro H-RTK F9P Series (Centimeter Precision RTK)

| Model: Standard Rover Puck | Model: Helical "Tower" Antenna |
| :---: | :---: |
| ![Holybro H-RTK F9P Rover](https://holybro.com/cdn/shop/files/12017-1_c32292af-f7e9-4c08-9748-210cc2d5ab65.jpg?v=1762850170&width=1080) | ![Holybro H-RTK F9P Helical](https://holybro.com/cdn/shop/files/12018-8_90f250d5-bcd1-4469-828a-74114e32bd7d.jpg?v=1762850203&width=1080) |
| *Figure 9: H-RTK F9P with flat dual-band patch antenna.* | *Figure 10: H-RTK F9P with cylindrical Quadrifilar Helix Antenna.* |

| Parameter | Specification |
| :--- | :--- |
| **Chipset** | **u-blox ZED-F9P** Multi-Band RTK Engine |
| **Frequency Bands** | **Dual-Band Concurrent**: GPS (L1C/A, L2C), GLONASS (L1OF, L2OF), Galileo (E1B/C, E5b), BeiDou (B1I, B2I) |
| **Position Accuracy** | **Centimeter-level: 1.0 cm + 1 ppm CEP** (with RTK Base Station RTCM3 corrections) |
| **Refresh Rate** | Up to 25 Hz |
| **Price** | **\$180 – \$250 USD (Rover only) / \$400+ USD (Base + Rover Set)** |

* **Pros**: Survey-grade centimeter precision. Eliminates drift completely. Dual-band resolves ionospheric delays in real time.
* **Cons**: High cost, requires a dedicated stationary ground base station sending RTCM corrections over telemetry radio, higher weight.
* **Verdict**: **For aerial surveying, photogrammetry, and precision landing missions.**

---

## 3. Price vs. Specification Comparison Matrix

| Model | Chipset | GNSS Bands | Constellations | Refresh Rate | Compass | Dimensions / Weight | Est. Price | Recommended Use |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **GY-NEO6MV2** | u-blox 6M | L1 | 1 (GPS only) | 1–5 Hz | None | $25\times35\text{ mm}$ / 18g | \$8 | Bench testing / Academic only |
| **Beitian BN-880** | u-blox M8N | L1 | 2 concurrent | 10 Hz | QMC5883L | $28\times28\times10\text{ mm}$ / 28g | \$18 | Budget 450mm builds |
| **Matek M10-5883** | u-blox M10 | L1 | **4 concurrent** | **18 Hz** | QMC5883L | $20\times20\times6.5\text{ mm}$ / **4.5g** | **\$28** | **Best All-Round Standard** |
| **Holybro Micro M10**| u-blox M10 | L1 | **4 concurrent** | **18 Hz** | IST8310 | $26\times26\times8\text{ mm}$ / 13g | \$35 | High-reliability quadcopters |
| **Holybro M10 Puck** | u-blox M10 | L1 | **4 concurrent** | **18 Hz** | IST8310 | $\varnothing 50\times15\text{ mm}$ / 32g | \$45 | Mast-mounted F450 builds |
| **Holybro H-RTK F9P** | u-blox F9P | **L1 + L2/L5** | 4 multi-band | 25 Hz | IST8310 | $\varnothing 55\times18\text{ mm}$ / 49g | \$210 | Centimeter survey & mapping |

---

## 4. Antenna Physics: Flat Ceramic Patch vs. Helical "Tower"

### 4.1 Flat Ceramic Patch Antennas (18x18mm vs. 25x25mm)

![Ceramic GPS Patch Antenna](https://5.imimg.com/data5/RU/MI/MY-20545300/ceramic-gps-patch-antenna-500x500.jpg)
*Figure 11: Standard 25x25mm ceramic patch antenna showing top silver metallization and central feed pin.*

- **Radiation Pattern**: Right-Hand Circular Polarization (**RHCP**). Radiation pattern is hemispherical, directed perpendicular to the ceramic face toward the sky.
- **Ground Plane Dependency**: A ceramic patch antenna **requires a conductive ground plane** beneath it (the PCB copper pour).
  - A $25\times25\text{ mm}$ patch on a $35\times35\text{ mm}$ ground plane delivers **$>3.5\text{ dBi}$** zenith gain.
  - An $18\times18\text{ mm}$ patch delivers **$\sim 1.5\text{ to }2.0\text{ dBi}$** zenith gain.
  - A $12\times12\text{ mm}$ micro-patch delivers **$<0\text{ dBi}$** gain (often struggling to track satellites below $25^\circ$ elevation).
- **Behavior During Drone Maneuvers**: When the drone banks heavily ($>30^\circ$ pitch/roll), the antenna face tilts away from zenith, causing a temporary $3\text{ – }6\text{ dB}$ signal drop. For hovering or gentle navigation on a 450mm drone, this is completely acceptable.

---

### 4.2 Quadrifilar Helix Antennas (QHA / Helical "Tower")

The cylindrical "tower" or stubby antenna seen on enterprise platforms (Figure 10) is a **Quadrifilar Helix Antenna (QHA)**:
- **Design**: Four helical wire conductors wrapped in a cylindrical helix inside a plastic radome.
- **Key Physics**:
  1. **Omnidirectional Hemispherical Pattern**: QHAs maintain consistent RHCP gain from zenith ($90^\circ$) all the way down to near the horizon ($10^\circ$).
  2. **No Ground Plane Required**: Performance is independent of the size of the drone frame or PCB ground plane.
  3. **Attitude Invariance**: When the drone rolls or pitches aggressively at high speed, the helical antenna continues receiving horizon satellites without signal fades.
- **Trade-off**: Heavier, more fragile in a crash, costs significantly more (\$40–\$80 for antenna alone).

---

## 5. The "Tower" (Elevated Standoff Mast): Why Multirotors Need It

One of the most noticeable features of F450, DJI Phantom, and Pixhawk drones is the **round saucer/puck mounted on a 10–14 cm carbon fiber folding mast**:

| Folding GPS Mast Standoff | Mast Installed on Multirotor |
| :---: | :---: |
| ![Foldable GPS Mast Down](https://www.getfpv.com/media/catalog/product/cache/f37f40fac2a7f4f1216822a65073de58/f/o/foldable-gps-mast-down.jpg) | ![GPS Folding Mount on Drone](https://drones.altigator.com/images/accessories/fpv_gps_folding_mount.jpg) |
| *Figure 12: CNC aluminum & carbon fiber folding mast.* | *Figure 13: Mast elevating GPS & compass 12 cm above the frame.* |

The reasons for elevating the GPS module are governed by electromagnetic physics:

### 5.1 Reason 1: Compass Magnetic Isolation (Ampère's Law & Toilet-Bowling)

This is the **number one reason** the module is placed on a mast.

On a 450mm quadcopter with four motors pulling $15\text{ A}$ each during a climb:
- Total current flowing through the central Power Distribution Board (PDB) and battery leads is $I \approx 60\text{ A}$.
- According to **Ampère’s Circuital Law**, the magnetic field $B$ at a distance $r$ from a current-carrying conductor is:
  $$B = \frac{\mu_0 \cdot I}{2 \pi \cdot r}$$
  where $\mu_0 = 4\pi \times 10^{-7}\text{ T}\cdot\text{m/A}$.

**Evaluating the induced distortion field:**
- **At $r = 2\text{ cm}$ ($0.02\text{ m}$)** (compass placed directly on the flight controller stack above the PDB):
  $$B_{\text{distort}} = \frac{4\pi \times 10^{-7} \times 60}{2\pi \times 0.02} = \frac{2 \times 10^{-7} \times 60}{0.02} = 600\ \mu\text{T} = 6.0\text{ Gauss}$$
- The Earth’s natural geomagnetic field is only **$30\text{ to }50\ \mu\text{T}$ ($0.3\text{ to }0.5\text{ Gauss}$)**!
- **Result**: The motor current creates a magnetic distortion **12 to 20 times stronger than Earth's magnetic field**, completely blinding the compass.
- **At $r = 14\text{ cm}$ ($0.14\text{ m}$)** (atop a folding mast):
  $$B_{\text{distort}} = \frac{2 \times 10^{-7} \times 60}{0.14} \approx 85.7\ \mu\text{T}$$
  With paired forward/return battery wiring canceling out net field, residual magnetic distortion drops by **$>96\%$** at the top of the mast.

> [!CAUTION]
> **The "Toilet-Bowling" Effect**: If compass yaw is corrupted by motor throttle, the flight controller assumes it is facing one direction while it is actually pointed elsewhere. When the position controller tries to counter horizontal drift, it pitches along the wrong heading vector, causing the drone to fly in an expanding circular orbit that rapidly destabilizes.

---

### 5.2 Reason 2: RF EMI Near-Field Isolation

The flight controller stack generates substantial high-frequency electromagnetic interference (EMI):
- **Microcontroller System Clock**: ESP32-S3 CPU running at $240\text{ MHz}$ produces square-wave harmonics at $480\text{ MHz}$, $720\text{ MHz}$, $960\text{ MHz}$, $1200\text{ MHz}$, $1440\text{ MHz}$, and **$1680\text{ MHz}$** (directly adjacent to the GPS L1 band at $1575.42\text{ MHz}$).
- **MicroSD SPI Clock**: Switching at $20\text{ MHz}$ radiates broadband edge harmonics.
- **Switching Regulators (BECs)**: 5V/3.3V buck converters pulsing at $500\text{ kHz} - 2\text{ MHz}$ radiate ripple through power lines.
- **RF Transmitters**: Nearby 2.4 GHz RC control links and 5.8 GHz analog/digital video transmitters create strong RF desensitization.

Raising the GPS module $12\text{ cm}$ moves the ceramic patch out of the near-field reactive zone, improving Carrier-to-Noise ratio ($C/N_0$) by **$6\text{ to }12\text{ dB-Hz}$**.

---

### 5.3 Reason 3: Horizon Line-of-Sight & LiPo Blocking

A 4S/6S $3000\text{–}5000\text{ mAh}$ LiPo battery is a dense rectangular brick of metal foils and electrolyte.
- If the GPS sits flush on the top plate next to or under the battery, the battery blocks satellite signals across $90^\circ\text{ to }180^\circ$ of the horizon.
- The elevated mast elevates the antenna above the battery's silhouette, maintaining a clear $360^\circ$ view down to $10^\circ$ elevation.

---

## 6. Recommendations: 450mm vs. Sub-250mm Quads

```
                    FRAME SCALE MOUNTING COMPARISON
                    
       450mm F450 / Camera Rig                 250mm FPV / Mini Quad
       -----------------------                 ---------------------
           (O) GPS Puck                            [===] Patch (15-30 deg tilt)
            |                                         \  TPU Mount
            |  10-14cm Carbon Mast                     \
      +-----------+                                +-----------+
      | LiPo / FC |                                |  LiPo/FC  |
      +-----------+                                +-----------+
     /             \                              /             \
    Motor         Motor                          Motor         Motor
   (9-10" Props)                                (5" Props)
   High current, spacious.                      Crash resilient, cramped.
```

### Recommendation for Your 450mm Drone:
1. **Module**: **u-blox MAX-M10S with Compass**
   - **Top Choice**: **Holybro M10 GPS Puck** or **Matek M10-5883** installed inside a puck shell.
   - **Budget Alternative**: **Beitian BN-880** (\$18).
2. **Mounting**: **10–14 cm Aluminum/Carbon Folding Mast**
   - Screw the base of the mast into the top carbon/glass-fiber center plate.
   - Fold down for transport; lock upright for flight.
   - Wire routing: Twist the 4–6 conductor cable (TX, RX, SCL, SDA, 5V, GND) along the carbon shaft to prevent loop antenna induction.

### Strategy for Smaller Quads (e.g., 250mm or sub-250g):
- A folding mast is too fragile for fast-flying mini quads that endure frequent crashes.
- Mount a compact **Matek M10-5883 (4.5g)** on a **flexible TPU rear tail mount**, tilted forward $20^\circ\text{ to }30^\circ$ (so the antenna sits level when the drone cruises forward).

---

## 7. Hardware & Firmware Integration for ESP32-S3 (`FC_v01`)

### 7.1 Electrical Wiring Hookup

The GPS module requires two independent digital buses:
1. **Serial UART** (For GNSS position, velocity, and time packets)
2. **I2C Bus** (For magnetometer heading data)

```
  ESP32-S3 Flight Controller                     GPS + Compass Module
  +-----------------------+                     +--------------------+
  |                       |                     |                    |
  |  GPIO 9 (UART1_TX)   ---------------------> | RX (Serial Data In)|
  |  GPIO 10 (UART1_RX)  <--------------------- | TX (Serial Data Out|
  |                       |                     |                    |
  |  GPIO 14 (I2C0_SCL)  ---------------------> | SCL (Compass Clock)|
  |  GPIO 21 (I2C0_SDA)  <--------------------> | SDA (Compass Data) |
  |                       |                     |                    |
  |  5V / 3.3V Output     ---------------------> | VCC (Main Power)   |
  |  GND                  ---------------------> | GND (System Ground)|
  +-----------------------+                     +--------------------+
```

*Note: The magnetometer operates as a standard I2C slave alongside the BMP180 barometer on your existing `I2C0` bus.*

---

### 7.2 FreeRTOS Task Architecture (`FC_v01`)

To comply with `FC_v01` architecture rules:
- **Zero dynamic memory allocation** during flight loops.
- Centralized task parameters declared in `include/config.h`.

```
                  FC_v01 GNSS SENSOR DAQ PIPELINE
                  
   UART Hardware FIFO
           |
     [Interrupt ISR]
           |
     RingBuffer<uint8_t, 512> (Layer 3 Middleware IPC)
           |
           v
   TASK_GPS_DAQ (10 Hz, Priority 2, Layer 4 Core Sensor)
     - State-machine binary parser: UBX-NAV-PVT
     - Extract Lat, Lon, Height, VelN, VelE, VelD, sAcc, vAcc, FixType
           |
           v
     RingBuffer<GpsNavData, 10>
           |
           +---------------------------------------+
           |                                       |
           v                                       v
   TASK_EKF_HORIZONTAL (50-100 Hz)          TASK_SD_LOGGER (10 Hz)
   - Fuses Accel XY + GPS Position/Vel      - Blackbox CSV/Binary Dump
   - Outputs World-Frame Pos & Vel
```

### 7.3 Protocol Selection: UBX Binary vs. NMEA

| Feature | NMEA 0183 (`$GNGGA`, `$GNRMC`) | u-blox UBX (`UBX-NAV-PVT`) |
| :--- | :--- | :--- |
| **Data Format** | Plaintext ASCII strings | Packed binary structures |
| **Parsing Overhead** | High (String scanning, `atof()`, delimiter splitting) | **Near zero** (State machine checksum + `memcpy` into struct) |
| **Bandwidth** | ~400–600 bytes per fix update | **92 bytes per fix update** |
| **Information Density** | Fragmented across 3–4 sentences | **Complete navigation solution in 1 message** |
| **Update Rate Capability**| Strains baud rate above 5 Hz | Runs cleanly at **10–18 Hz @ 115200 baud** |

**Recommendation**: Configure the u-blox receiver via firmware boot commands to disable all NMEA output and enable solely **`UBX-NAV-PVT`** at **115,200 baud** and **10 Hz**.
