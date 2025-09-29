<div align="center">
	<h1>⚡ ESP8266/ESP32 Offline Power Uptime Monitor</h1>
	<p>
		<img src="https://img.shields.io/badge/platform-ESP8266%20%7C%20ESP32-blue?style=flat-square" alt="Platform">
		<img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="License">
		<img src="https://img.shields.io/badge/made%20with-%E2%9D%A4%EF%B8%8F%20by%20anbuinfosec-ff69b4?style=flat-square" alt="Credit">
	</p>
	<p><b>Self-contained, sensorless, offline power uptime logger with a beautiful web dashboard.</b></p>
</div>

---

<p align="center">
	<img src="https://raw.githubusercontent.com/anbuinfosec/espPowerManagement/main/.github/dashboard-preview.png" alt="Dashboard Preview" width="600"/>
</p>

---

This project implements a self-contained ESP8266/ESP32 system that detects electricity availability by monitoring its own power state. It logs uptime, downtime, outages, and serves a local web dashboard with graphs.


## ✨ Features

- **Automatic power detection** (no sensors needed)
- **Uptime & downtime tracking**
- **Daily & weekly analytics**
- **ESP-hosted web dashboard** (HTML + Chart.js)
- **Data storage in LittleFS** (7–30 days history)
- **Data persistence across reboots**
- **Optional LED/buzzer alert** on power restore


## 🛠️ Hardware

- ESP8266 (NodeMCU / Wemos D1 Mini) or ESP32
- USB charger (mains powered)
- No external sensors, RTC, or SD card required



## 🚀 Usage

1. **Clone/download** this repo.
2. **Default WiFi credentials:**
	- **SSID:** `Power House`
	- **Password:** `anbuinfosec123`
	- You can change these from the web dashboard after flashing.
3. **Flash to ESP8266/ESP32:**
	 - **On PC:** Use PlatformIO or Arduino IDE as usual.
	 - **On Android:**
		 1. Install [ArduinoDroid](https://play.google.com/store/apps/details?id=name.antonsmirnov.android.arduinodroid2) from Google Play Store.
		 2. Copy the project folder to your Android device (e.g., in `ArduinoDroid/sketchbook/`).
		 3. Open ArduinoDroid, open the project, and install all required libraries (see links above).
		 4. Connect your ESP8266/ESP32 to your phone/tablet via OTG cable.
		 5. Select the correct board and port in ArduinoDroid settings.
		 6. Compile and upload the sketch.
4. **Access** the ESP's IP address in your browser for the dashboard.



## 📁 File Structure

- `src/main.cpp` — Main firmware code
- `data/` — LittleFS data folder (if needed)
- `README.md` — This file
- `CHANGELOG.md` — Version history and changes


## 📦 Libraries Required

- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP (ESP32)](https://github.com/me-no-dev/AsyncTCP) / [ESPAsyncTCP (ESP8266)](https://github.com/me-no-dev/ESPAsyncTCP)
- [LittleFS](https://github.com/earlephilhower/arduino-esp8266littlefs) (ESP8266) / [LittleFS_esp32](https://github.com/lorol/LITTLEFS)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)


## 🏗️ GitHub Actions: Automatic Build & Release

On every GitHub release, this project automatically builds and uploads pre-compiled firmware binaries for both ESP8266 and ESP32:

- `esp8266_framework.bin`
- `esp32_framework.bin`

You can download these files from the "Assets" section of each release on the [GitHub Releases page](../../releases).

**To flash:**
- Use [esphome-flasher](https://github.com/esphome/esphome-flasher), [ESPTool](https://github.com/espressif/esptool), or your favorite tool to upload the `.bin` file to your device.

**Workflow summary:**
- On every release, GitHub Actions builds both environments and attaches the binaries to the release automatically.


## 📝 License

<details>
<summary><strong>MIT License</strong></summary>

Copyright (c) 2025 anbuinfosec

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

</details>

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


---

<div align="center" style="color:#888; font-size:1.1em; margin-top:2em;">
Made with ❤️ by <a href="https://github.com/anbuinfosec" target="_blank"><b>@anbuinfosec</b></a>
</div>
