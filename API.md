# 🌐 API Documentation

## Web Server Endpoints

The ESP8266 runs a web server on port 80 with the following endpoints:

---

## 📄 Pages (GET Requests)

### Home Page
**Endpoint:** `/`  
**Method:** `GET`  
**Description:** Main dashboard with power history and statistics  
**Response:** HTML page with Bootstrap 5 styling

**Features:**
- Complete power event history table
- Color-coded badges (ON=Green, OFF=Red)
- Statistics for Today, 7-day, 15-day, and Monthly
- Current uptime display
- Navigation to other pages
- Clear logs button

**Example:**
```
http://192.168.1.100/
```

---

### ESP Stats Page
**Endpoint:** `/stats`  
**Method:** `GET`  
**Description:** Complete ESP8266 system information and power statistics  
**Response:** HTML page with system details

**Information Displayed:**
- **Hardware:** Chip ID, Flash Size, Free Heap, CPU Frequency
- **Software:** SDK Version, Boot Version, Sketch Size
- **Storage:** SPIFFS Total/Used/Free
- **Network:** WiFi Status, SSID, IP, MAC, RSSI, Gateway, DNS
- **System:** Current Time, Uptime, Reset Reason
- **Power Stats:** Today, 7-day, 15-day, Monthly summaries

**Example:**
```
http://192.168.1.100/stats
```

---

### Configuration Page
**Endpoint:** `/config`  
**Method:** `GET`  
**Description:** WiFi and AP configuration form  
**Response:** HTML form with current settings

**Form Fields:**
- WiFi SSID (text input)
- WiFi Password (password input)
- Fallback AP SSID (text input)
- Fallback AP Password (password input, min 8 chars)

**Example:**
```
http://192.168.1.100/config
```

---

## 🔧 Actions (POST/GET Requests)

### Save Configuration
**Endpoint:** `/save`  
**Method:** `POST`  
**Description:** Save WiFi configuration and restart device  
**Content-Type:** `application/x-www-form-urlencoded`

**Parameters:**
| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `ssid` | string | No | WiFi SSID to connect |
| `pass` | string | No | WiFi password |
| `apssid` | string | No | Fallback AP SSID (default: ESPPowerMonitor) |
| `appass` | string | No | Fallback AP password (min 8 chars) |

**Response:** HTML confirmation page with auto-restart

**Example:**
```bash
curl -X POST http://192.168.1.100/save \
  -d "ssid=MyWiFi&pass=MyPassword&apssid=MyESP&appass=12345678"
```

**Behavior:**
1. Saves configuration to EEPROM
2. Displays success message
3. Waits 2 seconds
4. Restarts ESP8266

---

### Clear Logs
**Endpoint:** `/clear`  
**Method:** `GET`  
**Description:** Delete all power logs and reset statistics  
**Response:** HTML confirmation with auto-redirect

**Actions Performed:**
- Deletes `/power_log.txt`
- Deletes `/last_on.txt`
- Resets all statistics
- Redirects to home page after 2 seconds

**Example:**
```
http://192.168.1.100/clear
```

---

## 📊 Data Format

### Log File Format
**File:** `/power_log.txt`  
**Location:** SPIFFS filesystem  
**Format:** Plain text, one entry per line

**Entry Structure:**
```
[TYPE] timestamp duration
```

**Fields:**
- `[TYPE]`: Event type - `[ON]` or `[OFF]`
- `timestamp`: Unix epoch time (seconds since 1970-01-01)
- `duration`: Duration in seconds (for OFF events, time power was off)

**Example:**
```
[ON] 1730534400 0
[OFF] 1730534400 3600
[ON] 1730538000 0
[OFF] 1730542800 7200
```

### Last Power-On File
**File:** `/last_on.txt`  
**Location:** SPIFFS filesystem  
**Format:** Single timestamp value

**Content:**
```
1730534400
```
- Contains last power-on timestamp
- Updated hourly
- Used to calculate power-off duration on next boot

---

## 🔐 EEPROM Configuration

### Memory Layout
**Total Size:** 512 bytes

| Address | Size | Purpose |
|---------|------|---------|
| 0-63 | 64 bytes | WiFi SSID |
| 64-127 | 64 bytes | WiFi Password |
| 128-191 | 64 bytes | AP SSID |
| 192-255 | 64 bytes | AP Password |
| 250 | 1 byte | Configuration Flag (0x01) |
| 256-259 | 4 bytes | Last Monthly Reset Timestamp |

### Configuration Flag
- **Value:** `0x01` (1) = Configuration saved
- **Purpose:** Indicates valid configuration exists
- **Location:** Address 250

---

## 🌐 Network Modes

### Station Mode (WiFi Client)
**Activated when:** Valid WiFi credentials exist in EEPROM

**Connection Process:**
1. Read SSID and password from EEPROM
2. Attempt connection for 15 seconds
3. LED blinks during connection
4. On success: Get IP via DHCP, LED solid
5. On failure: Switch to AP Mode

**IP Address:** Assigned by DHCP (e.g., 192.168.1.100)

### Access Point Mode (Fallback)
**Activated when:** 
- No WiFi credentials saved
- WiFi connection fails

**Default Settings:**
- SSID: `ESPPowerMonitor`
- Password: `12345678`
- IP Address: `192.168.4.1`
- Subnet: `255.255.255.0`

**LED:** Solid ON when AP active

---

## 🕐 Time Synchronization

### NTP Configuration
**Servers:**
- Primary: `pool.ntp.org`
- Secondary: `time.nist.gov`

**Timezone:** UTC+6 (Asia/Dhaka)  
**Offset:** 6 hours (21600 seconds)

**Sync Process:**
- Attempts: 30 maximum
- Interval: 500ms between attempts
- Total timeout: ~15 seconds
- Success indicator: `time(nullptr) >= 100000`

**To Change Timezone:**
```cpp
configTime(YOUR_OFFSET*3600, 0, "pool.ntp.org", "time.nist.gov");
```

---

## 📈 Statistics Calculation

### Time Periods

**Today:**
- Start: Midnight (00:00:00) of current day
- End: Current time
- Calculation: `now - (hours*3600 + minutes*60 + seconds)`

**Last 7 Days:**
- Start: 7 days ago from now
- End: Current time
- Calculation: `now - (7 * 86400)`

**Last 15 Days:**
- Start: 15 days ago from now
- End: Current time
- Calculation: `now - (15 * 86400)`

**This Month:**
- Start: 1st day of current month at 00:00:00
- End: Current time
- Calculation: `now - ((day-1)*86400 + hours*3600 + minutes*60 + seconds)`

### Accumulation Logic
```cpp
for each log entry:
  if timestamp >= period_start:
    if type is [OFF]:
      accumulate OFF duration
    if type is [ON]:
      accumulate ON duration (future use)
```

---

## 🔄 Monthly Auto-Reset

### Reset Trigger
Checks on every boot if month has changed

**Logic:**
```cpp
if (current_year != last_reset_year) OR (current_month != last_reset_month):
  - Delete /power_log.txt
  - Delete /last_on.txt
  - Save current timestamp to EEPROM
  - Print "Monthly reset performed!"
```

**Storage:** Last reset timestamp stored at EEPROM address 256-259 (4 bytes)

---

## 💾 File Operations

### SPIFFS Functions

**Initialize:**
```cpp
SPIFFS.begin();
```

**Write Log Entry:**
```cpp
File f = SPIFFS.open("/power_log.txt", "a");  // Append mode
f.println("[ON] 1730534400 0");
f.close();
```

**Read All Logs:**
```cpp
File f = SPIFFS.open("/power_log.txt", "r");
while(f.available()) {
  String line = f.readStringUntil('\n');
  // Process line
}
f.close();
```

**Delete File:**
```cpp
SPIFFS.remove("/power_log.txt");
```

---

## 🎨 HTTP Response Format

### HTML Structure
All pages use Bootstrap 5 and follow this structure:

```html
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width,initial-scale=1.0'>
  <link href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css' rel='stylesheet'>
  <title>Page Title</title>
  <style>/* Custom styles */</style>
</head>
<body>
  <!-- Navbar -->
  <nav class='navbar navbar-expand-lg navbar-dark bg-dark'>...</nav>
  
  <!-- Content -->
  <div class='container mt-4'>
    <div class='card p-4'>...</div>
  </div>
  
  <!-- Footer -->
  <footer class='text-center'>
    Made with ❤️ by <a href='https://github.com/anbuinfosec'>@anbuinfosec</a>
  </footer>
  
  <script src='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js'></script>
</body>
</html>
```

---

## 🔧 Arduino Functions

### Main Functions

**Setup:**
```cpp
void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  configTime(...);
  WiFi.begin(...) or WiFi.softAP(...);
  server.on("/", handleRoot);
  server.begin();
}
```

**Loop:**
```cpp
void loop() {
  server.handleClient();
  // Hourly update of last_on timestamp
}
```

### HTTP Handlers

**Home:**
```cpp
void handleRoot() {
  server.send(200, "text/html", getHTML());
}
```

**Stats:**
```cpp
void handleStats() {
  server.send(200, "text/html", statsPage());
}
```

**Config:**
```cpp
void handleConfig() {
  server.send(200, "text/html", configPage());
}
```

**Save:**
```cpp
void handleSave() {
  // Get form data
  // Save to EEPROM
  // Send response
  // Restart
}
```

**Clear:**
```cpp
void handleClear() {
  SPIFFS.remove(logFile);
  server.send(200, "text/html", html_with_redirect);
}
```

---

## 📝 Usage Examples

### Get Current Statistics
```bash
# Using curl to fetch home page
curl http://192.168.1.100/

# Using wget
wget -O dashboard.html http://192.168.1.100/
```

### Get System Info
```bash
curl http://192.168.1.100/stats
```

### Update WiFi Configuration
```bash
curl -X POST http://192.168.1.100/save \
  -d "ssid=NewWiFi" \
  -d "pass=NewPassword" \
  -d "apssid=MyESP8266" \
  -d "appass=SecurePass123"
```

### Clear All Logs
```bash
curl http://192.168.1.100/clear
```

---

## 🛠️ Development Notes

### Adding New Endpoints

1. **Define handler function:**
```cpp
void handleCustom() {
  String html = pageHeader("Custom Page", "custom");
  html += "<div class='card p-4'>Custom content</div>";
  html += pageFooter();
  server.send(200, "text/html", html);
}
```

2. **Register in setup():**
```cpp
server.on("/custom", handleCustom);
```

3. **Add to navbar (optional):**
```cpp
nav += "<li class='nav-item'><a class='nav-link' href='/custom'>Custom</a></li>";
```

---

## 📊 Performance Metrics

### Typical Response Times
- Home Page: 80-150ms
- Stats Page: 100-200ms
- Config Page: 50-100ms
- Save Config: 2000ms (includes restart)
- Clear Logs: 50-100ms

### Memory Usage
- Flash: ~300KB (sketch)
- RAM: ~20KB (runtime)
- SPIFFS: Variable (log size)
- EEPROM: 512 bytes

---

Made with ❤️ by [@anbuinfosec](https://github.com/anbuinfosec)
