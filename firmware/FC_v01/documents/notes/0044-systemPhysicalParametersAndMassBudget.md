---
id: "0044"
title: System Physical Parameters, Mass Budget & Plant Inertia Estimation
author: Drone Research Team / Developer
date: 2026-10-25
time: 18:38:21 +07:00
tags: ["#zettelkasten", "#hardware", "#physical_parameters", "#mass_budget", "#inertia", "#plant_model", "#fc_v01"]
references:
  - "[[0000-firmwareStructureLayerDecision]]"
  - "[[0034-motorDriverLedcHalQuadxActuatorTaskArchitecture]]"
---

# Zettel 0044: System Physical Parameters, Mass Budget & Plant Inertia Estimation

## 1. Metadata & Scope
- **ID**: `0044`
- **Author**: Drone Research Team / Developer
- **Date**: 2026-10-25
- **Time**: 18:38:21 +07:00
- **Status**: Recorded
- **References**: `[Note 0000]`, `[Note 0034]`

---

## 2. Mass Breakdown & Measured Weight Budget

Direct bench scale measurements of the individual vehicle subsystems for the 450 mm quadrotor:

| Component Subsystem | Measured Mass (g) | Mass (kg) | Fraction of Total (%) |
| :--- | :---: | :---: | :---: |
| **Airframe + 4x Motors + 4x Propellers** | $707.2\,\text{g}$ | $0.7072\,\text{kg}$ | $77.24\%$ |
| **Flight Battery (LiPo)** | $173.7\,\text{g}$ | $0.1737\,\text{kg}$ | $18.97\%$ |
| **Flight Controller (`FC_v01` PCB + ESP32-S3)** | $20.4\,\text{g}$ | $0.0204\,\text{kg}$ | $2.23\%$ |
| **RC Receiver (`FS-iA6B` Decased / Harness)** | $14.3\,\text{g}$ | $0.0143\,\text{kg}$ | $1.56\%$ |
| **Total Takeoff Mass ($m_{\text{total}}$)** | $\mathbf{915.6\,\text{g}}$ | $\mathbf{0.9156\,\text{kg}}$ | $\mathbf{100.00\%}$ |

---

## 3. Geometric & Frame Constants

- **Airframe Configuration**: Quad-X (symmetric $45^\circ$ arm geometry)
- **Diagonal Wheelbase ($D$)**: $450\,\text{mm} = 0.450\,\text{m}$
- **Motor Radius from C.G. ($R = D/2$)**: $0.225\,\text{m}$
- **Perpendicular Pitch / Roll Moment Arm ($L = R \cdot \sin(45^\circ)$)**:
  $$L = \frac{0.225}{\sqrt{2}} \approx 0.1591\,\text{m}$$

---

## 4. Derived Plant Inertia Estimation

Using the standard cylindrical-hub and distributed tip-mass approximation for a 450 mm symmetric quadrotor ($I_{xx} \approx I_{yy} \approx \frac{1}{12} m_{\text{total}} D^2$):

- **Roll & Pitch Principal Moments of Inertia ($I_{xx}, I_{yy}$)**:
  $$I_{xx} \approx I_{yy} \approx \frac{1}{12} \cdot 0.9156\,\text{kg} \cdot (0.450\,\text{m})^2 \approx 0.01545\,\text{kg}\cdot\text{m}^2$$
- **Yaw Principal Moment of Inertia ($I_{zz} \approx I_{xx} + I_{yy}$ by perpendicular axis theorem)**:
  $$I_{zz} \approx 2 \cdot 0.01545 \approx 0.03090\,\text{kg}\cdot\text{m}^2$$

---

## 5. Cross-References & Traceability
- **Supersedes**: N/A (Baseline physical parameter recording)
- **Related Notes**:
  - `[[0000-firmwareStructureLayerDecision]]`: Layer architecture overview
  - `[[0034-motorDriverLedcHalQuadxActuatorTaskArchitecture]]`: Actuator output mapping and mixer structure
