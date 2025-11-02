
# Changelog

All notable changes to this project will be documented in this file.

## [v2.1.0] - 2025-11-02

### 🔄 Major Changes
- **Removed WiFi Repeater Mode**
  - Simplified to Station mode with AP fallback
  - Removed NAT/NAPT functionality
  - Removed lwip/napt.h and lwip/dns.h dependencies
  - Cleaner, more stable WiFi management

### 🐛 Bug Fixes
- Removed complex AP+STA simultaneous mode
- Fixed WiFi mode switching logic
- Improved auto-reconnect reliability
- AP only shows when needed (fallback mode)

### 📝 Documentation
- Updated README.md to reflect new WiFi architecture
- Removed REPEATER_MODE.md (no longer applicable)
- Updated FEATURES.md with simplified network flow
- Changed default AP SSID to `ESPPowerMonitor`

### 🔧 Technical Changes
- WiFi.mode() now uses WIFI_STA or WIFI_AP (not WIFI_AP_STA)
- Simplified setup() WiFi initialization
- Removed NAT enable code
- Removed custom IP configuration for AP

## [v2.0.0] - 2025-11-02

### ✨ Major Features Added
- **Complete Power Statistics Dashboard**
  - Today's power off/on time tracking
  - Last 7 days statistics
  - Last 15 days statistics  
  - Monthly overview with auto-reset
- **ESP8266 Stats Page** (`/stats`)
  - Complete hardware information (Chip ID, Flash, Heap, CPU)
  - SPIFFS usage statistics
  - WiFi connection details (SSID, IP, RSSI, MAC, Gateway, DNS)
  - Power statistics integration
  - Reset reason and uptime tracking
- **Monthly Auto-Reset Feature**
  - Automatically clears logs at month start
  - Prevents SPIFFS overflow
  - Keeps data fresh and relevant
  - Stored in EEPROM for persistence
- **Enhanced Power Logging**
  - Duration calculation for both ON and OFF events
  - Improved log format with duration storage
  - Better timestamp accuracy with NTP sync
  - Hourly timestamp updates
- **Modern Web Interface**
  - Bootstrap 5 styling with responsive navbar
  - Active page indicators in navigation
  - Color-coded event badges (Green=ON, Red=OFF)
  - Responsive mobile design
  - Loading spinners for async operations
  - Alert boxes for status messages
  - Improved button styling and layout

### 🔧 Improvements
- Extended EEPROM to 512 bytes for additional data
- Stores last monthly reset timestamp
- Hourly timestamp updates for accuracy
- Increased WiFi timeout to 15 seconds
- Better serial output for debugging
- Form validation on config page
- Password input fields with placeholders
- Minimum 8-character password enforcement
- Fallback to default AP SSID if empty
- Arduino IDE compatible version included
- Vector-based log parsing for better memory management
- Improved string concatenation for stability

### 🐛 Bug Fixes
- Fixed duration calculation accuracy
- Proper first boot scenario handling
- More reliable NTP synchronization (30 attempts, 15 seconds)
- Improved timeout handling
- Fixed table formatting issues
- Better responsive design
- Improved button layouts
- Fixed ternary operator issues in Arduino IDE
- Proper struct initialization

### 🔐 Security
- Password fields hidden in web forms
- EEPROM-based credential storage
- AP password minimum length (8 chars)
- Input validation on forms

### 📚 Documentation
- Complete README.md update
- Detailed SETUP_GUIDE.md
- Comprehensive FEATURES.md
- Updated CHANGELOG.md
- Arduino IDE installation instructions

## [v1.0.1] - 2025-09-30
- AP-only mode: ESP always shows its own WiFi for web panel
- WiFi AP is now open (no password required)
- Removed NTP/configTime (no internet in AP-only mode)
- Improved LED logic: blinks only on WiFi connect attempts, stays ON in AP mode
- Minor code and documentation cleanups

## [v1.0.0] - 2025-09-30
- Initial public release
- ESP8266/ESP32 offline power uptime monitor
- Modern web dashboard with Chart.js
- WiFi credentials changeable via web UI
- Data logging and analytics (uptime, downtime, outages)
- LittleFS storage, persistent logs
- GitHub Actions CI/CD for firmware builds
- Default WiFi SSID: `Power House`
- Default WiFi Password: `anbuinfosec123`
- ArduinoJson 7.x+ compatibility (no deprecation warnings)

---

## Version Roadmap

### [v2.1.0] - Planned
- [ ] Chart.js graphs for visual power trends
- [ ] Export logs to CSV functionality
- [ ] JSON API endpoints
- [ ] Real-time updates via WebSocket
- [ ] MQTT support for remote monitoring
- [ ] Email/Telegram notifications

### [v3.0.0] - Future
- [ ] OTA (Over-The-Air) updates
- [ ] Multi-language support
- [ ] Mobile app integration
- [ ] Advanced analytics with predictions
- [ ] Cloud backup integration
- [ ] Multi-device monitoring

---

Made with ❤️ by [@anbuinfosec](https://github.com/anbuinfosec)
