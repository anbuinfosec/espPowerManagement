# 🚀 Quick Setup Guide

## Prerequisites
- ESP8266 board (NodeMCU, Wemos D1 Mini, etc.)
- USB cable
- PlatformIO or Arduino IDE

## Step-by-Step Installation

### 1. Flash the Firmware

#### Using PlatformIO (Recommended)
```bash
cd espPowerManagement
pio run -t upload
pio device monitor
```

#### Using Arduino IDE
1. Open `ESP8266_PowerMonitor_Arduino.ino`
2. Select board: **Tools → Board → ESP8266 → NodeMCU 1.0 (ESP-12E Module)**
3. Select port: **Tools → Port → [Your ESP Port]**
4. **Board Settings:**
   - Flash Size: 4MB (FS:2MB OTA:~1019KB)
   - CPU Frequency: 80 MHz
   - Upload Speed: 115200
5. Click **Upload** (or Ctrl+U)
6. Open **Serial Monitor** (115200 baud)

### 2. First Connection

1. **Power on ESP8266**
2. **Connect to WiFi AP:**
   - SSID: `ESPPowerMonitor`
   - Password: `12345678`
3. **Open browser:** `http://192.168.4.1`

### 3. Configure WiFi

1. Click **"Config"** button
2. Enter your WiFi credentials:
   - WiFi SSID: Your router name
   - WiFi Password: Your router password
3. (Optional) Change AP settings:
   - Fallback AP SSID: Custom name
   - Fallback AP Password: Min 8 characters
4. Click **"Save & Restart"**

### 4. Find Your ESP's IP

Check serial monitor output:
```
Syncing time.......... Done!
Connecting to WiFi: YourSSID
Connected! IP: 192.168.1.100  ← Your IP here
Power ON logged at: 2025-11-02 10:30:45
Web server started!
```

### 5. Access Dashboard

Open browser: `http://[YOUR_ESP_IP]`

Example: `http://192.168.1.100`

## 📱 Web Interface

### Home Page (`/`)
- View complete power history
- See statistics (Today, 7-day, 15-day, Monthly)
- Access all features

### ESP Stats (`/stats`)
- Hardware information
- WiFi details
- System status
- Memory usage

### Config (`/config`)
- Change WiFi credentials
- Modify AP settings
- Restart device

### Clear Logs (`/clear`)
- Delete all logs
- Reset statistics
- Fresh start

## 🔧 Customization

### Change Timezone
Edit in `main.cpp`:
```cpp
configTime(6*3600, 0, "pool.ntp.org", "time.nist.gov");
```
Replace `6` with your UTC offset.

### Change LED Pin
```cpp
#define LED_PIN 2  // Change this
```

### Change Default AP
```cpp
String apSSID="ESPPowerMonitor";  // Your custom name
String apPASS="12345678";          // Min 8 chars
```

## 🐛 Troubleshooting

### Can't see AP
- Wait 30 seconds after power on
- Check ESP8266 has power
- Look for `ESPPowerMonitor` in WiFi list

### Can't connect to WiFi
- Verify SSID and password are correct
- Check router is 2.4GHz (ESP8266 doesn't support 5GHz)
- Ensure router DHCP is enabled

### Time not syncing
- Requires internet connection
- Wait up to 30 seconds
- Check NTP servers are accessible

### Web page not loading
- Verify correct IP address
- Try `http://` not `https://`
- Ensure device is on same network

### Logs not saving
- Check serial monitor for SPIFFS errors
- Verify free space with `/stats` page
- Try clearing logs and restart

## 📊 Understanding the Data

### Event Types
- **[ON]** (Green badge): Power restored
- **[OFF]** (Red badge): Power lost

### Statistics
- **Today**: From midnight to now
- **Last 7 Days**: Rolling 7-day window
- **Last 15 Days**: Rolling 15-day window
- **This Month**: From 1st of month to now

### Auto-Reset
- Logs automatically clear on 1st of each month
- Prevents SPIFFS overflow
- Keeps statistics relevant

## 🔐 Security Tips

1. **Change default AP password** (min 8 chars)
2. **Use strong WiFi passwords**
3. **Keep device on trusted network**
4. **Regularly check logs** for anomalies

## 📞 Support

- **GitHub Issues:** Report bugs
- **Serial Monitor:** Check for errors
- **LED Indicator:** 
  - Blinking: Connecting
  - Solid: Connected

## 🎯 Tips for Best Results

1. Use stable power supply (5V, 1A minimum)
2. Place ESP8266 near router for good signal
3. Check `/stats` page for system health
4. Monitor SPIFFS usage regularly
5. Back up logs before clearing (if needed)

---

Made with ❤️ by [@anbuinfosec](https://github.com/anbuinfosec)
