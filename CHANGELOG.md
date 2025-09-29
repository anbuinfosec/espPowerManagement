
# Changelog

## v1.0.1 (2025-09-30)
- AP-only mode: ESP always shows its own WiFi for web panel, never tries to connect to external WiFi
- WiFi AP is now open (no password required)
- Removed NTP/configTime (no internet in AP-only mode)
- Improved LED logic: blinks only on WiFi connect attempts, stays ON in AP mode
- Minor code and documentation cleanups

## v1.0.0 (2025-09-30)
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
