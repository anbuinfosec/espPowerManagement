<div align="center">
	<h1>⚡ ESP8266 Power Management Monitor</h1>
	<p>
		<img src="https://img.shields.io/badge/platform-ESP8266-blue?style=flat-square" alt="Platform">
		<img src="https://img.shields.io/badge/version-2.0.1-brightgreen?style=flat-square" alt="Version">
		<img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="License">
		<img src="https://img.shields.io/badge/made%20with-%E2%9D%A4%EF%B8%8F%20by%20anbuinfosec-ff69b4?style=flat-square" alt="Credit">
	</p>
	<p><b>Comprehensive power monitoring system with statistics, analytics, and beautiful Bootstrap 5 interface.</b></p>
</div>

---

> **Latest Update:** Simplified WiFi management - Station mode with AP fallback. Clean, reliable power monitoring with mobile-responsive Bootstrap 5 interface.

This project implements a self-contained ESP8266 system that automatically tracks power on/off events with precise timestamps, calculates power-off durations, and provides detailed statistics through a modern responsive web interface.


## ✨ Features

### 🔋 Power Monitoring
- **Automatic Power Detection** - No sensors needed, monitors own power state
- **Event Logging** - Records every power ON/OFF with timestamps
- **Duration Calculation** - Automatically calculates power-off time
- **Persistent Storage** - All data saved to SPIFFS filesystem

### 📊 Statistics & Analytics
- **Today's Stats** - Current uptime and power-off duration
- **7-Day Analysis** - Last week's power patterns
- **15-Day Analysis** - Two-week power usage trends
- **Monthly Overview** - Complete month statistics
- **Auto-Reset** - Automatically clears logs after each month

### 🌐 Modern Web Interface
- **Bootstrap 5 Design** - Responsive, mobile-friendly UI
- **Navigation Bar** - Quick access to all pages
- **Home Page** - Complete power history table + statistics
- **Stats Page** - Detailed ESP8266 system information
- **Config Page** - Easy WiFi configuration
- **Color-Coded Events** - Green badges for ON, Red for OFF

### ⚙️ Configuration
- **WiFi Station Mode** - Connects to your WiFi network
- **Fallback AP Mode** - Creates access point if WiFi fails
- **Web-Based Config** - No need to reflash for changes
- **Persistent Settings** - Configuration survives reboots
- **Auto-Reconnect** - Automatically reconnects to WiFi if disconnected

### 💡 Smart Features
- **LED Indicator** - Blinks while connecting, solid when ready
- **NTP Time Sync** - Asia/Dhaka timezone (UTC+6) ⭐ UPDATED
- **Hourly Updates** - Timestamp refreshed every hour
- **Monthly Reset** - Prevents SPIFFS overflow
- **Bootstrap 5 UI** - Fixed navbar toggle, mobile responsive ⭐ FIXED

## 🛠️ Hardware

- **ESP8266** (NodeMCU / Wemos D1 Mini / Generic ESP8266)
- **USB Cable** for programming
- **5V Power Supply** (mains powered for monitoring)
- **No external sensors, RTC, or SD card required**



## 🚀 Quick Start

### Installation Options

#### Option 1: Arduino IDE (Easiest)
1. **Open** `ESP8266_PowerMonitor_Arduino.ino` in Arduino IDE
2. **Select Board**: Tools → Board → ESP8266 → NodeMCU 1.0
3. **Select Port**: Tools → Port → [Your ESP Port]
4. **Upload** the sketch
5. **Open Serial Monitor** (115200 baud)

#### Option 2: PlatformIO (Recommended)
```bash
git clone https://github.com/anbuinfosec/espPowerManagement.git
cd espPowerManagement
pio run -t upload
pio device monitor
```

#### Option 3: Pre-compiled Binary
1. Download `.bin` file from [Releases](https://github.com/anbuinfosec/espPowerManagement/releases)
2. Flash using [ESPTool](https://github.com/espressif/esptool) or [ESP Flash Tool](https://www.espressif.com/en/support/download/other-tools)

### First Setup

1. **Power on ESP8266**
2. **Connect to WiFi AP:**
   - SSID: `ESPPowerMonitor`
   - Password: `12345678`
3. **Open browser:** `http://192.168.4.1`
4. **Configure WiFi:**
   - Click "Config" in navigation
   - Enter your WiFi credentials
   - Click "Save & Restart"
5. **Find IP:** Check Serial Monitor for assigned IP (in Station mode)
6. **Access Dashboard:** 
   - Station mode: `http://[YOUR_ESP_IP]`
   - AP mode: `http://192.168.4.1`



## 📁 Project Structure

```
espPowerManagement/
├── src/
│   └── main.cpp                    # PlatformIO source
├── ESP8266_PowerMonitor_Arduino.ino  # Arduino IDE version
├── platformio.ini                  # PlatformIO config
├── README.md                       # This file
├── SETUP_GUIDE.md                  # Detailed setup instructions
├── FEATURES.md                     # Complete feature documentation
├── CHANGELOG_v2.md                 # Version history
└── LICENSE                         # MIT License
```

## � Web Pages

### Home Page (`/`)
- Power history table with all ON/OFF events
- Color-coded badges (Green=ON, Red=OFF)
- Statistics dashboard showing:
  - Today's power off/on time
  - Last 7 days totals
  - Last 15 days totals
  - Monthly totals
- Current uptime and last updated timestamp

### ESP Stats Page (`/stats`)
- Complete hardware information
- Chip ID, Flash size, Free heap
- CPU frequency, SDK version
- SPIFFS usage statistics
- WiFi connection details
- IP address, MAC, RSSI
- Power statistics summary
- System uptime

### Config Page (`/config`)
- WiFi SSID and Password
- Fallback AP SSID and Password
- Form validation
- Auto-restart after save

### Clear Logs (`/clear`)
- Delete all power logs
- Reset statistics
- Auto-redirect to home

## 📦 Dependencies

**Built-in libraries (no installation needed):**
- ESP8266WiFi
- ESP8266WebServer
- FS (SPIFFS)
- EEPROM
- time.h
- vector (C++ STL)


## � Screenshots

### Home Page
- Beautiful Bootstrap 5 interface
- Responsive navbar with active indicators
- Color-coded event badges
- Comprehensive statistics table
- Real-time uptime display

### Stats Page
- Complete system diagnostics
- Hardware specifications
- Network information
- Power statistics integration

### Config Page
- Clean form design
- Password masking
- Input validation
- Success/error notifications

## 🔧 Customization

### Change Timezone
```cpp
configTime(6*3600, 0, "pool.ntp.org", "time.nist.gov");
```
Replace `6` with your UTC offset (6 = UTC+6 for Asia/Dhaka).

### Change LED Pin
```cpp
#define LED_PIN 2  // Built-in LED (GPIO2)
```

### Change Default AP Credentials
```cpp
String apSSID="ESPPowerMonitor";
String apPASS="12345678";  // Min 8 characters
```

### Adjust WiFi Timeout
```cpp
while(WiFi.status()!=WL_CONNECTED&&millis()-start<15000)
```
Change `15000` to desired timeout in milliseconds.


## � Troubleshooting

### Can't Connect to AP
- Wait 30 seconds after power on
- Look for `ESPPowerMonitor` in WiFi list
- Check ESP has stable 5V power supply
- If WiFi configured, ESP will be in Station mode (no AP visible)

### WiFi Connection Fails
- Verify SSID and password are correct
- Ensure router is 2.4GHz (ESP8266 doesn't support 5GHz)
- Check router DHCP is enabled

### Time Not Syncing
- Requires internet connection
- Wait up to 30 seconds after boot
- Check NTP servers are accessible

### Web Page Not Loading
- Verify correct IP address
- Use `http://` not `https://`
- Ensure same network connection

### Logs Not Saving
- Check SPIFFS via `/stats` page
- Verify free space available
- Try clearing logs and restart

## 📖 Documentation

- **[QUICK_FIX.md](QUICK_FIX.md)** - 🚨 Quick troubleshooting guide
- **[TROUBLESHOOTING_WIFI_TIME.md](TROUBLESHOOTING_WIFI_TIME.md)** - Complete troubleshooting guide
- **[SETUP_GUIDE.md](SETUP_GUIDE.md)** - Detailed setup instructions
- **[FEATURES.md](FEATURES.md)** - Complete feature documentation
- **[API.md](API.md)** - API endpoints and technical reference
- **[CHANGELOG.md](CHANGELOG.md)** - Version history and updates

## 🎯 Use Cases

- **Home Power Monitoring** - Track power outages and reliability
- **Office Equipment** - Monitor critical device uptime
- **Data Centers** - Server power monitoring and logging
- **Remote Locations** - Off-grid power system tracking
- **Solar Systems** - Track power availability patterns
- **UPS Testing** - Verify backup power effectiveness

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.


---

<div align="center" style="color:#888; font-size:1.1em; margin-top:2em;">
Made with ❤️ by <a href="https://github.com/anbuinfosec" target="_blank"><b>@anbuinfosec</b></a>
</div>
