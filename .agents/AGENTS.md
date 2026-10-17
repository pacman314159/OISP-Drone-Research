# Repository Architecture & Multi-Module Rules

Global guidelines for the **00-drone-research-2026** workspace. Sub-modules (`firmware/FC_v01/`, `analyzers/`) maintain their own dedicated `.agents/` rules, skills, and project tracking.

## 1. Documentation & Hardware References
- System architecture specifications: `system_architecture/System Architecture v01.pdf`.
- Research papers: `research_docs/`.
- Hardware component datasheets: `datasheets/`.
- Architectural decisions & notes: `firmware/FC_v01/documents/notes/`.

## 2. Sensor Processing & Analysis Scripts (`/analyzers`)
- Barometer & Magnetometer data processing scripts must use FFT noise filtering routines (`baro_fft.py`, `imu_fft.py`, `mag_fft.py`).

## 3. Security
- Never commit OAuth secrets, tokens, or `credentials.json` files to Git. All secrets must remain in `.agents/credentials.json` (ignored by `.gitignore`).

## 4. Build Execution Policy
- Do NOT execute `pio run` or PlatformIO build/flash commands automatically; the developer triggers all builds manually.
