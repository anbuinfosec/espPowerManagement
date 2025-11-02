# 📋 Features Documentation

## Complete Feature List

### 🔋 Power Monitoring

#### Automatic Power Event Detection
- **Power-ON Detection**: Automatically logged when ESP8266 boots
- **Power-OFF Detection**: Calculated from last known timestamp
- **Duration Tracking**: Precise calculation of power interruption time
- **Persistent Storage**: All events saved to SPIFFS filesystem

#### Event Logging Format
```
[ON]  timestamp duration
[OFF] timestamp duration
```

Example:
```
[ON]  1730534400 0
[OFF] 1730534400 3600
[ON]  1730538000 0
```

### 📊 Statistics & Analytics

#### Real-Time Statistics
- **Current Uptime**: Time since last boot
- **Today's Stats**: 
  - Total power-off time today (from midnight)
  - Current online time
- **7-Day Rolling Window**:
  - Total power-off time in last 7 days
  - Total power-on time in last 7 days
- **15-Day Rolling Window**:
  - Total power-off time in last 15 days
  - Total power-on time in last 15 days
- **Monthly Overview**:
  - Total power-off time this month
  - Total power-on time this month

#### Auto-Reset Feature
- **Monthly Reset**: Automatically clears logs on 1st of each month
- **Purpose**: 
  - Prevents SPIFFS overflow
  - Keeps statistics relevant
  - Fresh start each month
- **Implementation**: Checks month change on boot
- **Stored in EEPROM**: Last reset timestamp persists

### 🌐 Web Interface

#### Home Page (`/`)

**Features:**
- Complete power history table
- Color-coded event badges
- Duration display for each event
- Statistics dashboard
- Navigation buttons
- Current timestamp display

**Table Columns:**
1. Event # (sequential numbering)
2. Event Type (ON/OFF with color badge)
3. Timestamp (human-readable)
4. Duration (formatted: XdXhXmXs)

**Statistics Section:**
- Today's power off/on time
- Last 7 days summary
- Last 15 days summary
- Monthly totals

#### ESP Stats Page (`/stats`)

**Hardware Information:**
- Chip ID (hexadecimal)
- Flash Chip ID
- Flash Size & Real Flash Size
- Free Heap Memory
- CPU Frequency (MHz)
- SDK Version
- Boot Version & Mode
- Sketch Size & Free Space
- Reset Reason & Info

**Filesystem Information:**
- SPIFFS Total Bytes
- SPIFFS Used Bytes
- SPIFFS Free Bytes
- Usage percentage

**WiFi Information (Station Mode):**
- Connection Status
- SSID
- IP Address
- Gateway IP
- Subnet Mask
- DNS Server
- MAC Address
- RSSI (signal strength)

**WiFi Information (AP Mode):**
- AP IP Address
- AP MAC Address
- Connected clients count

**System Information:**
- Current Time (NTP synchronized)
- System Uptime

#### Config Page (`/config`)

**Configuration Options:**
1. **Wi-Fi SSID** (required)
   - Your router's network name
   - Used for station mode connection
   
2. **Wi-Fi Password** (optional)
   - Your router's password
   - Leave empty for open networks
   
3. **Fallback AP SSID** (required)
   - Custom AP name
   - Default: `ESPPowerMonitor`
   - Used when WiFi connection fails
   
4. **Fallback AP Password** (optional)
   - AP password (min 8 characters)
   - Default: `12345678`
   - Empty = open network

**Features:**
- Form validation
- Password input fields (hidden)
- Auto-restart after save
- Confirmation message

#### Clear Logs Page (`/clear`)

**Functionality:**
- Deletes `/power_log.txt`
- Deletes `/last_on.txt`
- Resets all statistics
- Auto-redirects to home page
- Confirmation message

### 💡 LED Indicator

**States:**
- **Blinking (250ms)**: Connecting to WiFi
- **Solid ON**: Connected or AP mode active
- **Pin**: GPIO 2 (built-in LED on most boards)

**Logic:**
```
Boot → LED OFF
Connecting WiFi → LED Blinking
Connected → LED ON
Failed → Start AP → LED ON
```

### 🕐 Time Management

#### NTP Synchronization
- **Primary Server**: `pool.ntp.org`
- **Secondary Server**: `time.nist.gov`
- **Timezone**: UTC+6 (Asia/Dhaka)
- **Sync on Boot**: Up to 30 attempts (15 seconds)
- **Retry Logic**: 500ms between attempts

#### Time Display
- **Format**: `YYYY-MM-DD HH:MM:SS`
- **Example**: `2025-11-02 14:30:45`
- **Used For**:
  - Event timestamps
  - Statistics calculations
  - Monthly reset checks
  - Display on all pages

### 💾 Data Persistence

#### EEPROM Layout (512 bytes)
```
Address   Size    Purpose
0-63      64B     Wi-Fi SSID
64-127    64B     Wi-Fi Password
128-191   64B     AP SSID
192-255   64B     AP Password
250       1B      Configuration Flag (0x01)
256-259   4B      Last Monthly Reset Timestamp
```

#### SPIFFS Files
1. **`/power_log.txt`**
   - Format: `[TYPE] timestamp duration\n`
   - Append-only file
   - Contains all power events
   - Size: Dynamic (auto-managed)

2. **`/last_on.txt`**
   - Format: `timestamp`
   - Single timestamp value
   - Updated hourly
   - Used for duration calculation

#### Data Integrity
- **Hourly Updates**: Last-on timestamp updated every hour
- **Atomic Writes**: Complete file operations
- **Error Handling**: Graceful failure modes
- **Auto-Recovery**: Handles missing/corrupt files

### 🔌 Network Management

#### Station Mode (Primary)
- **Auto-Connect**: Uses saved credentials
- **Timeout**: 20 seconds
- **Auto-Reconnect**: Every 30 seconds if disconnected
- **Fallback**: AP mode on failure

#### Access Point Mode (Fallback)
- **Auto-Start**: If WiFi fails or no credentials
- **Default SSID**: `ESPPowerMonitor`
- **Default Password**: `12345678`
- **IP Address**: `192.168.4.1`
- **Subnet**: `255.255.255.0`

#### Connection Flow
```
Boot
  ↓
Check EEPROM for WiFi credentials
  ↓
Credentials Found?
  ↓ Yes                    ↓ No
Connect WiFi (20s)     Start AP Mode
  ↓
Success?
  ↓ Yes                ↓ No
Station Mode       Fallback AP Mode
  ↓                      ↓
Auto-reconnect     Always accessible
every 30s          at 192.168.4.1
```

### 🎨 User Interface

#### Bootstrap 5 Styling
- **Responsive Grid**: Mobile-first design
- **Cards**: Content containers
- **Tables**: Striped, bordered data display
- **Buttons**: Color-coded actions
- **Badges**: Status indicators
- **Forms**: Clean input fields

#### Color Scheme
- **Success/Green**: Power ON, Save button
- **Danger/Red**: Power OFF, Clear button
- **Primary/Blue**: Config, Info buttons
- **Info/Cyan**: Stats button
- **Muted/Gray**: Footer, timestamps

#### Responsive Design
- **Mobile**: Single column layout
- **Tablet**: Optimized spacing
- **Desktop**: Full width tables
- **All Devices**: Touch-friendly buttons

### ⚙️ Configuration Options

#### Compile-Time Settings
```cpp
#define LED_PIN 2              // LED pin
#define EEPROM_SIZE 512        // EEPROM size
#define WIFI_SSID_ADDR 0       // SSID address
#define WIFI_PASS_ADDR 64      // Password address
#define AP_SSID_ADDR 128       // AP SSID address
#define AP_PASS_ADDR 192       // AP Password address
#define FLAG_ADDR 250          // Config flag
#define LAST_RESET_ADDR 256    // Reset timestamp
```

#### Runtime Settings
- All WiFi credentials
- AP settings
- Timezone (via NTP offset)

### 🔒 Security Features

#### Password Protection
- **Minimum Length**: 8 characters for AP
- **Hidden Input**: Password fields masked
- **Storage**: EEPROM (plain text)

#### Access Control
- **No Authentication**: Open web interface
- **Local Only**: No remote access by default
- **Network Isolation**: AP mode isolated

#### Recommendations
1. Change default AP password
2. Use strong WiFi passwords
3. Keep device on trusted network
4. Regularly monitor access logs
5. Update firmware regularly

### 📈 Performance

#### Memory Usage
- **Flash**: ~300KB (sketch size)
- **RAM**: ~20KB (runtime)
- **SPIFFS**: Dynamic log storage
- **EEPROM**: 512 bytes

#### Speed
- **Boot Time**: ~5-10 seconds
- **Web Response**: <100ms
- **Log Write**: <50ms
- **Stats Calculation**: <200ms

#### Scalability
- **Max Events**: Limited by SPIFFS
- **Max History**: ~30 days (typical)
- **Monthly Reset**: Ensures longevity

### 🐛 Error Handling

#### WiFi Errors
- Connection timeout → AP mode
- Lost connection → Maintain operation
- Invalid credentials → AP mode

#### Time Sync Errors
- Failed NTP → Continue with system time
- Timeout → Log warning, retry on reboot

#### File System Errors
- Missing files → Create on demand
- Full SPIFFS → Monthly reset
- Corrupt data → Skip invalid entries

#### Power Errors
- Rapid power cycles → All logged
- Long offline → Duration calculated
- Timestamp drift → Corrected on boot

### 🎯 Use Cases

#### Home Power Monitoring
- Track power outages
- Calculate downtime
- Monthly reliability stats

#### Office/Business
- Monitor critical equipment uptime
- Generate reliability reports
- Track power quality

#### Data Center
- Server uptime monitoring
- Power backup testing
- UPS effectiveness tracking

#### Remote Locations
- Off-grid power monitoring
- Solar system tracking
- Generator runtime logging

### 🔄 Maintenance

#### Regular Tasks
- Check SPIFFS usage monthly
- Backup important logs
- Update WiFi credentials as needed
- Monitor LED for issues

#### Troubleshooting
- Reset via config page
- Clear logs if SPIFFS full
- Check serial monitor for errors
- Verify time sync on boot

---

Made with ❤️ by [@anbuinfosec](https://github.com/anbuinfosec)
