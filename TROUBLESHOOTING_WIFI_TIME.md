# 🔧 Troubleshooting Guide - WiFi & Time Issues

## Common Problems & Solutions

### ❌ Problem 1: Time Shows 1970-01-01

**Symptoms:**
- Web interface shows dates from 1970
- Serial Monitor shows "1970-01-01 00:00:00"
- Power logs have incorrect timestamps

**Root Cause:**
- NTP (Network Time Protocol) sync failed
- Usually means NO INTERNET CONNECTION

**Solutions:**

#### Step 1: Check WiFi Connection
Open Serial Monitor (115200 baud) and look for:
```
✓ Connected to WiFi!
  Station IP: 192.168.1.xxx
```

If you see:
```
✗ Failed to connect to WiFi
  Status: 6
```
Then WiFi is NOT connected (see Problem 2 below)

#### Step 2: Check NTP Sync
Look for:
```
Syncing time from NTP............ ✓ Success!
Current time: 2025-11-02 14:30:45
```

If you see:
```
⚠ Skipping NTP sync (no internet connection)
Time will show as 1970 until WiFi connects
```
Then the device knows there's no internet.

#### Step 3: Fix Time Issue

**Option A: Connect to WiFi** (Recommended)
1. Connect to ESP's AP: `ESPPowerMonitor` (password: `12345678`)
2. Open `http://192.168.4.1`
3. Go to Config page
4. Enter correct WiFi SSID and password
5. Click Save & Restart
6. Wait 30 seconds
7. Check Serial Monitor for successful connection

**Option B: Wait for Auto-Retry**
- The device now automatically retries NTP sync every 60 seconds
- Once WiFi connects, time will sync automatically
- Check Serial Monitor for: `✓ NTP sync successful`

**Option C: Check Internet Connection**
- Verify your router has internet access
- Try visiting a website from another device
- Check if firewall blocks NTP (port 123 UDP)

---

### ❌ Problem 2: WiFi Not Connecting After Configuration

**Symptoms:**
- Saved WiFi credentials via Config page
- ESP restarts but doesn't connect
- Serial shows: `✗ Failed to connect to WiFi`
- Status code 6 (WL_DISCONNECTED) or other error codes

**Root Causes & Solutions:**

#### Cause 1: Incorrect Credentials ⚠️ MOST COMMON

**Check for:**
- ❌ Wrong SSID (case-sensitive!)
- ❌ Wrong password (case-sensitive!)
- ❌ Extra spaces before/after SSID or password
- ❌ Hidden SSID (ESP8266 has issues with hidden networks)

**Solution:**
1. Connect to AP: `192.168.4.1`
2. Open Serial Monitor to see what's saved:
   ```
   SSID from EEPROM: MyWiFi (length: 6)
   Password length: 8
   ```
3. Verify SSID length matches expected
4. Re-enter credentials carefully
5. **Copy-paste if possible** to avoid typos

**Pro Tip:** The latest version now trims whitespace automatically!

#### Cause 2: Router Security Settings

**Check your router for:**
- ❌ MAC address filtering enabled
- ❌ WPA3-only mode (ESP8266 needs WPA2)
- ❌ 5GHz-only network (ESP8266 only supports 2.4GHz)
- ❌ Hidden SSID
- ❌ Too many connected devices (DHCP pool full)

**Solution:**
- Use WPA2-PSK (AES) encryption
- Enable 2.4GHz band
- Make SSID visible
- Check DHCP has available addresses
- Temporarily disable MAC filtering to test

#### Cause 3: Signal Strength

**Check RSSI value:**
```
Signal: -45 dBm  ← Excellent
Signal: -65 dBm  ← Good
Signal: -75 dBm  ← Fair
Signal: -85 dBm  ← Poor (may disconnect)
```

**Solution:**
- Move ESP closer to router
- Use WiFi extender
- Change router channel (avoid congestion)
- Use external antenna if available

#### Cause 4: Router Compatibility

**Some routers have issues with:**
- Enterprise WPA (not supported)
- Captive portals (hotel/public WiFi)
- Client isolation mode enabled

**Solution:**
- Use home/standard WiFi
- Disable client isolation
- Use personal hotspot for testing

---

## 🔍 Debugging Steps

### 1. Serial Monitor Debug Output

The updated code provides detailed debugging:

```
=== WiFi Configuration ===
SSID from EEPROM: MyHomeWiFi (length: 11)
Password length: 12

Connecting to WiFi: 'MyHomeWiFi'
....
✓ Connected to WiFi!
  IP: 192.168.1.105
  Gateway: 192.168.1.1
  Signal: -52 dBm

=== Time Synchronization ===
Syncing time from NTP....................... ✓ Success!
Current time: 2025-11-02 14:30:45
```

### 2. WiFi Status Codes

| Code | Status | Meaning |
|------|--------|---------|
| 0 | WL_IDLE_STATUS | Not started |
| 1 | WL_NO_SSID_AVAIL | **SSID not found** |
| 3 | WL_CONNECTED | ✓ Connected |
| 4 | WL_CONNECT_FAILED | **Wrong password** |
| 6 | WL_DISCONNECTED | Disconnected/timeout |

### 3. Check Saved Configuration

Via Serial Monitor:
```
Saving WiFi config:
  SSID: MyWiFi (length: 6)
  Password: [SET] (length: 12)
```

Via Web Interface (`/stats` page):
- **WiFi Mode:** Shows current mode (Access Point/Station)
- **WiFi Status:** Shows connection state
- **Connected To:** Shows actual SSID connected to (Station mode)
- **Station IP:** Shows assigned IP address (Station mode)
- **AP Info:** Shows AP details (AP mode)

---

## 📋 Complete Reset Procedure

If nothing works, do a complete reset:

### Method 1: Clear EEPROM
```cpp
// Add to setup() temporarily:
for(int i=0;i<512;i++){
  EEPROM.write(i,0);
}
EEPROM.commit();
```

### Method 2: Access Clear Logs
1. Connect to AP: `192.168.4.1`
2. Click "Clear Logs"
3. Restart ESP
4. Reconfigure WiFi

### Method 3: Re-flash Firmware
1. Upload sketch again
2. Use "Erase Flash: All Flash Contents" in Arduino IDE
3. This clears all saved data

---

## ✅ Verification Checklist

After configuration, verify:

- [ ] Serial Monitor shows WiFi connected
- [ ] Serial Monitor shows valid IP address
- [ ] Serial Monitor shows successful NTP sync
- [ ] Time on web interface is correct (not 1970)
- [ ] Can access via assigned IP (Station mode) or 192.168.4.1 (AP mode)
- [ ] Stats page shows correct WiFi mode (Station or Access Point)
- [ ] Power logs have correct timestamps

---

## 🆕 What's Fixed in Latest Version

### WiFi Improvements:
✅ Whitespace trimming on save
✅ Whitespace trimming on load
✅ Better Serial debugging output
✅ Increased connection timeout (20 seconds)
✅ WiFi status code reporting
✅ SSID/password length verification

### Time Sync Improvements:
✅ NTP sync AFTER WiFi connects (was before)
✅ Automatic NTP retry every 60 seconds
✅ Invalid time detection (skips logging if time=1970)
✅ Extended NTP timeout (60 attempts = 30 seconds)
✅ Better error messages
✅ Time validation before logging events

### New Features:
✅ Station mode with AP fallback
✅ Dual network access
✅ Enhanced debug output
✅ Better error handling

---

## 🌍 Timezone Configuration

Current: **Asia/Dhaka (UTC+6)**

To change timezone, edit this line in `setup()`:
```cpp
configTime(6*3600, 0, "pool.ntp.org", "time.nist.gov");
```

### Common Timezones:
```cpp
// UTC+6 (Bangladesh - current)
configTime(6*3600, 0, "pool.ntp.org", "time.nist.gov");

// UTC+5:30 (India)
configTime(5.5*3600, 0, "pool.ntp.org", "time.nist.gov");

// UTC+8 (China, Singapore)
configTime(8*3600, 0, "pool.ntp.org", "time.nist.gov");

// UTC+9 (Japan, Korea)
configTime(9*3600, 0, "pool.ntp.org", "time.nist.gov");

// UTC-5 (Eastern Time)
configTime(-5*3600, 0, "pool.ntp.org", "time.nist.gov");

// UTC-8 (Pacific Time)
configTime(-8*3600, 0, "pool.ntp.org", "time.nist.gov");
```

---

## 📞 Still Having Issues?

### Quick Fixes:
1. **Power cycle** - Unplug for 10 seconds
2. **Check power supply** - Use quality 5V 1A+ adapter
3. **Try different WiFi** - Test with phone hotspot
4. **Update router firmware** - May fix compatibility

### Get Help:
- **Serial Monitor** - Share full output
- **GitHub Issues** - Report bugs with details
- **Stats Page** - Screenshot network info

### Useful Info to Share:
- ESP8266 board model (NodeMCU/Wemos/Generic)
- Router brand/model
- Router security mode (WPA2/WPA3)
- Serial Monitor output
- WiFi status code from debug output

---

**Made with ❤️ by @anbuinfosec**
