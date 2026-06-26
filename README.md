<h1 align="center">🌡️ Smart Environment Hub</h1>
<p align="center"><b>ESP32-based environmental monitoring station with adaptive RGB feedback</b></p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32-00979D?style=flat&logo=espressif&logoColor=white" />
  <img src="https://img.shields.io/badge/Simulated%20on-Wokwi-1A1A1A?style=flat&logo=arduino&logoColor=white" />
  <img src="https://img.shields.io/badge/Language-C%2B%2B%20(Arduino)-blue?style=flat" />
  <img src="https://img.shields.io/badge/License-MIT-green?style=flat" />
</p>

<p align="center">
  <a href="https://wokwi.com/projects/461371419163577345"><b>▶️ Run the live simulation on Wokwi</b></a>
</p>

---

## 📌 Overview

**Smart Environment Hub** is an ESP32 simulation that continuously monitors **temperature, humidity, and ambient light**, then communicates the environment's state through two channels: a rotating **16x2 I2C LCD display** and a **color-coded RGB LED**. A single push button lets the user adjust how long each LCD screen stays visible — a small UX touch that turns a simple sensor demo into a configurable monitoring tool.

**Smart Environment Hub**, ESP32 üzerinde çalışan; sıcaklık, nem ve ortam ışığını sürekli ölçen bir simülasyon projesidir. Ölçüm sonuçları iki kanaldan iletilir: dönüşümlü ekranlar gösteren bir **16x2 I2C LCD** ve duruma göre renk değiştiren bir **RGB LED**. Tek bir buton, LCD ekranlarının görüntülenme süresini ayarlamaya yarar.

---

## ✨ Features

- 🌡️ Real-time temperature & humidity readings via **DHT22**
- 💡 Ambient light sensing via **LDR**, converted to an approximate lux value
- 📟 Auto-rotating LCD screens (Humidity → Temperature → Light) with live status labels
- 🚦 **RGB LED status indicator** that reacts to both light and temperature conditions
- 🔘 Debounced push button to toggle LCD refresh interval (2s ↔ 4s)
- 📊 Serial monitor logging every 3 seconds for debugging / data inspection

---

## 🧰 Hardware Components

| Component | Role | ESP32 Pin |
|---|---|---|
| ESP32 DevKit V1 | Main controller | — |
| DHT22 | Temperature & humidity sensor | GPIO 15 (data), 3V3, GND |
| Photoresistor (LDR) | Ambient light sensor | GPIO 34 (analog in), 3V3, GND |
| 16x2 LCD (I2C, addr `0x27`) | Status display | GPIO 21 (SDA), GPIO 22 (SCL), 3V3, GND |
| RGB LED (common cathode) | Visual status indicator | GPIO 25 (R), 26 (G), 27 (B) — via 220Ω resistors |
| Push button | Display interval toggle | GPIO 18 (`INPUT_PULLUP`) |

---

## ⚙️ How It Works

**1. Sensing**
Every loop cycle, the DHT22 is read for temperature/humidity, and the LDR's raw analog value (0–4095) is mapped to an approximate **0–1000 lux** range and clamped to that bound.

**2. Status classification**

| Metric | Thresholds |
|---|---|
| Humidity | ≤ 60% → `LOW` · > 60% → `HIGH` |
| Temperature | < 0°C → `VERY LOW` · 0–30°C → `NORMAL` · > 30°C → `HIGH` |
| Light | < 10 lux → `KARANLIK` (dark) · ≥ 10 lux → `AYDINLIK` (bright) |

**3. RGB feedback logic**

| Condition | LED Color |
|---|---|
| Dark environment | 🔵 Blue |
| Bright + temp < 0°C | 🩵 Cyan |
| Bright + 0°C ≤ temp ≤ 30°C | 🟢 Green |
| Bright + 30°C < temp ≤ 40°C | 🟡 Yellow |
| Bright + temp > 40°C | 🔴 Red |

**4. Display rotation**
The LCD cycles through three screens — Humidity, Temperature, Light — every `lcdSure` milliseconds (default **2000ms**).

**5. Button control**
A debounced (50ms) push button toggles `lcdSure` between **2000ms and 4000ms**, with a brief on-screen confirmation message before returning to the normal rotation.

**6. Serial logging**
Every 3 seconds, current humidity, temperature, and lux values are printed to the Serial Monitor at `115200` baud.

---

## 🔌 Circuit Diagram

Full wiring and component layout are available in the live simulation:

👉 **[Open in Wokwi](https://wokwi.com/projects/461371419163577345)**

<details>
<summary>📐 Pin connection summary</summary>

```
ESP32 D15  ──── DHT22 SDA
ESP32 3V3  ──── DHT22 VCC
ESP32 GND  ──── DHT22 GND

ESP32 3V3  ──── LDR VCC
ESP32 GND  ──── LDR GND
ESP32 D34  ──── LDR AO

ESP32 D25 ── 220Ω ── RGB LED (R)
ESP32 D26 ── 220Ω ── RGB LED (G)
ESP32 D27 ── 220Ω ── RGB LED (B)
RGB LED COM ──── GND

ESP32 D21 (SDA) ──── LCD SDA
ESP32 D22 (SCL) ──── LCD SCL
ESP32 3V3       ──── LCD VCC
ESP32 GND       ──── LCD GND

ESP32 D18 ──── Push Button ──── GND
```
</details>

---

## 📊 Sample Serial Output

```
Nem: 45.20% || Sicaklik: 23.10C || Isik: 412 lux
Nem: 45.00% || Sicaklik: 23.15C || Isik: 408 lux
LCD suresi degistirildi: 4 sn
```

---

## 🚀 Getting Started

**Run in simulation (no hardware required):**
1. Open the [Wokwi project link](https://wokwi.com/projects/461371419163577345)
2. Click ▶️ **Play** to start the simulation
3. Adjust the DHT22/LDR values in the simulator to see live LCD and RGB updates
4. Click the on-screen push button to toggle the display interval

**Run on real hardware:**
1. Wire the components per the [pin connection summary](#-circuit-diagram) above
2. Install the required libraries: `LiquidCrystal I2C`, `DHT sensor library for ESPx`
3. Flash `sketch.ino` to your ESP32 via Arduino IDE or PlatformIO
4. Open the Serial Monitor at `115200` baud to view live readings

---

## 📁 Project Structure

```
smart-environment-hub/
├── sketch.ino          # Main firmware logic
├── diagram.json         # Wokwi circuit/wiring definition
└── libraries.txt        # Required Arduino libraries
```

---

## 🛠️ Tech Stack

<p>
  <img src="https://img.shields.io/badge/ESP32-00979D?style=flat&logo=espressif&logoColor=white" />
  <img src="https://img.shields.io/badge/Arduino%20C%2B%2B-00979D?style=flat&logo=arduino&logoColor=white" />
  <img src="https://img.shields.io/badge/I2C-LCD%201602-blue?style=flat" />
  <img src="https://img.shields.io/badge/DHT22-Sensor-orange?style=flat" />
  <img src="https://img.shields.io/badge/Wokwi-Simulator-1A1A1A?style=flat" />
</p>

---

## 📄 License

This project is licensed under the **MIT License**.

---

## 🙋 Author
 
**Kübra Parmak**
- GitHub: [@KbrPrmk](https://github.com/KbrPrmk)
- Wokwi: [@kbrprmk](https://wokwi.com/makers/kbrprmk)
