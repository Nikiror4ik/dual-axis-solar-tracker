# Dual-Axis Solar Tracker with Wi-Fi Telemetry

An automated IoT-based dual-axis solar tracking system designed using **Arduino Uno**, **SparkFun**, and a **Python/Flask** backend for data logging.

## 📌 Features
- **Dual-Axis Tracking:** Calculates light intensity from 4 photoresistors (LDR) to adjust 2 servo motors (horizontal & vertical).
- **Wi-Fi Telemetry:** Transmits solar panel voltage and light levels over HTTP POST requests using AT commands.
- **Backend Logging:** Flask server receiving telemetry metrics on port 8000.
- **Safety Limits:** Includes software angles clamping, low-light night mode, and servo jitter protection (`attach`/`detach` logic).

## 🛠️ Hardware Used
- Arduino Uno R3
- SparkFun Wi-Fi Module
- LM1117-3.3V Step-Down Power Module
- 2x SG90 Micro Servo Motors
- 4x Light Dependent Resistors (LDR)
- Solar Panel + Voltage Divider Sensor
- 100µF Capacitor & Resistors (1kΩ, 10kΩ, 220Ω)

## 📁 Repository Structure
- `tracker_withTUTTO_public` — Arduino C++ tracking and telemetry source code.
- `app.py` — Python Flask server script.
- `photo_Solar` — Project photo.

---
*Developed as part of the Signals and Measurements.*
