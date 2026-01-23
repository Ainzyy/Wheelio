# WiFi Configuration Guide

## Overview

The Wheelio system now includes WiFi connectivity with automatic credential storage and recovery. The WiFi implementation uses:

- **Native ESP32 WiFi Library** - No external dependencies
- **NVS (Non-Volatile Storage)** - Persistent credential storage
- **Web Portal** - Browser-based configuration interface
- **Boot Button Trigger** - Easy WiFi portal access during runtime

---

## Features

### 1. **Automatic Reconnection**
- Device remembers the last connected WiFi network
- Credentials stored in ESP32 NVS (survives power loss/restart)
- Automatically connects on startup if credentials are saved

### 2. **Web Configuration Portal**
- Access via browser when device is in AP mode
- Simple form to enter SSID and password
- HTML interface responsive and user-friendly

### 3. **Runtime Portal Trigger**
- Press ESP32 boot button for 2+ seconds while running
- Opens WiFi configuration AP automatically
- Useful for switching networks without code changes

### 4. **Connection Monitoring**
- Real-time WiFi status tracking
- Signal strength (RSSI) monitoring
- Automatic reconnection on disconnection

---

## Configuration

### config.h Settings

```cpp
// --- WiFi Configuration ---
#define ENABLE_WIFI 1                     // Enable WiFi (1=on, 0=off)
#define WIFI_PORTAL_TIMEOUT_SEC 120       // Portal timeout in seconds
#define WIFI_RECONNECT_INTERVAL_MS 5000   // Reconnect check interval
#define PIN_BOOT_BUTTON 0                 // GPIO 0 (Boot button on ESP32)
#define WIFI_DEVICE_NAME "Wheelio-ESP32"  // WiFi AP name for portal
```

### Enabling/Disabling WiFi

In `src/main.cpp`, WiFi is automatically managed with:
```cpp
#if ENABLE_WIFI
    wifiHandler.begin();    // In setup()
    wifiHandler.update();   // In loop()
#endif
```

Enable WiFi by setting in config.h:
```cpp
#define ENABLE_WIFI 1
```

Disable WiFi by setting:
```cpp
#define ENABLE_WIFI 0
```

---

## Usage Scenarios

### Scenario 1: First Time Setup

1. **Upload firmware** to ESP32 (with ENABLE_WIFI = 1)
2. **Open Serial Monitor** (115200 baud)
3. **Press Boot Button** for 2 seconds
4. **Look for WiFi network** named "Wheelio-ESP32"
5. **Connect to network** from your phone/laptop
6. **Navigate to** 192.168.4.1 in browser
7. **Fill in WiFi credentials** and click "Save & Connect"
8. **Device reboots** and connects to the network

### Scenario 2: Switching Networks

**Method A - Using Boot Button:**
1. **Press boot button** for 2 seconds while device is running
2. **See "Wheelio-ESP32" WiFi network** appear
3. **Connect and configure** as in Scenario 1

**Method B - Over-the-Air Configuration (Future):**
- Will include MQTT/HTTP endpoints for remote network switching

### Scenario 3: Recovery After Power Loss

1. Device **automatically powers on**
2. Loads saved credentials from NVS
3. **Automatically connects** to last known network
4. **No user intervention required**

---

## Serial Output Examples

### Successful First-Time Connection

```
[WiFi] Initializing WiFi Handler...
[WiFi] Initializing WiFi...
[WiFi] No saved credentials found
[WiFi] Press boot button for 2 seconds to open configuration portal

-- (User presses boot button) --

[WiFi] Boot button pressed...
[WiFi] Boot button held for 2 seconds - opening portal
[WiFi] Starting WiFi configuration portal...
[WiFi] Creating soft AP for configuration...
[WiFi] Soft AP started: Wheelio-ESP32
[WiFi] AP IP address: 192.168.4.1
[WiFi] Connect to this network and navigate to 192.168.4.1
[WiFi] Web server started on port 80

-- (User enters WiFi credentials and submits) --

[WiFi] Saving credentials for SSID: MyNetwork
[WiFi] Connecting to new network...
..............
[WiFi] Successfully connected!
[WiFi] IP address: 192.168.1.100
```

### Reconnection After Disconnect

```
[WiFi] Connected!
[WiFi] IP: 192.168.1.100

-- (WiFi disconnected) --

[WiFi] Connection lost, attempting to reconnect...
.....
[WiFi] Connected!
[WiFi] IP: 192.168.1.100
```

### Saved Credentials Auto-Connect

```
[WiFi] Initializing WiFi Handler...
[WiFi] Initializing WiFi...
[WiFi] Found saved credentials for: MyNetwork
[WiFi] Attempting to connect to saved network...
...
[WiFi] Connected to saved network!
[WiFi] IP address: 192.168.1.100
```

---

## WiFiHandler API

### Methods

```cpp
// Initialize WiFi (call in setup())
void begin();

// Update WiFi connection (call in loop())
void update();

// Check if currently connected
bool isConnected() const;

// Get current WiFi status
WiFiStatus getStatus() const;  // Returns: WIFI_STATUS_DISCONNECTED, 
                               //          WIFI_STATUS_CONNECTING,
                               //          WIFI_STATUS_CONNECTED

// Get connected SSID
String getSSID() const;

// Get signal strength in dBm
int getRSSI() const;

// Manually start configuration portal
void startPortal();

// Check if boot button pressed (returns true when LOW)
bool isBootButtonPressed() const;

// Print status to Serial
void printStatus() const;
```

### Example Usage in Custom Code

```cpp
// In your application code
#if ENABLE_WIFI
    // Check if connected before sending data
    if (wifiHandler.isConnected()) {
        // Send data to cloud
        httpClient.POST("https://api.example.com/data", payload);
    }
    
    // Get signal strength
    int rssi = wifiHandler.getRSSI();
    Serial.print("Signal: ");
    Serial.print(rssi);
    Serial.println(" dBm");
    
    // Print full status
    wifiHandler.printStatus();
#endif
```

---

## Technical Details

### Credential Storage (NVS)

Credentials are stored in **ESP32's Non-Volatile Storage**:

```cpp
Preferences preferences;
preferences.begin("wheelio-wifi", false);
preferences.putString("ssid", ssid);
preferences.putString("password", password);
preferences.end();
```

**Advantages:**
- Survives power loss and reboots
- No need for hardcoded credentials
- Can store ~100+ networks worth of data

### Web Portal Server

When boot button is held:
1. Device switches to **AP Mode** (Access Point)
2. Creates WiFi network: **Wheelio-ESP32**
3. Hosts web server on **192.168.4.1**
4. Serves HTML form with SSID/password input
5. Posts credentials back to device
6. Switches back to **STA Mode** and connects

**Portal Timeout:** 120 seconds (configurable)

### Auto-Reconnection

Every 5 seconds (configurable), checks:
```cpp
if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();  // Uses stored credentials
}
```

---

## Troubleshooting

### Problem: Device Won't Connect to WiFi

**Solution:**
1. Check SSID and password are correct (case-sensitive)
2. Verify WiFi network is visible and accessible
3. Check serial output for error messages
4. Press boot button to reconfigure network
5. Verify signal strength (RSSI > -80 dBm recommended)

### Problem: Can't See "Wheelio-ESP32" Network

**Solution:**
1. Ensure boot button is held for **full 2 seconds**
2. Check serial output for "Soft AP started" message
3. Try disconnecting from other WiFi first
4. Restart ESP32 with `Reset` button and try again

### Problem: Portal Times Out

**Solution:**
1. Verify connection to Wheelio-ESP32 network
2. Check browser can reach 192.168.4.1
3. Try different browser (Chrome, Firefox, Safari)
4. Clear browser cache
5. Increase WIFI_PORTAL_TIMEOUT_SEC in config.h

### Problem: Device Remembers Old Network After Power Loss

**This is expected behavior.** To force new network:
1. Press boot button for 2 seconds
2. Reconfigure to desired network
3. New credentials will be saved and used on next boot

To **erase all saved credentials:**
```cpp
// Add this to setup() temporarily
preferences.begin("wheelio-wifi", false);
preferences.clear();
preferences.end();
```

---

## Security Considerations

### Current Implementation (MVP)

- **Open AP:** WiFi portal network has no password
- **Local Only:** Portal operates only in local AP mode
- **No HTTPS:** Web interface uses plain HTTP (acceptable for local AP)

### Future Enhancements (Phase 3+)

- [ ] Add password protection to AP
- [ ] Implement HTTPS certificate support
- [ ] Add WiFi encryption selection (WPA2/WPA3)
- [ ] Implement WiFi MAC filtering
- [ ] Add connection timeout protection

---

## Performance

### Memory Usage
- WiFi Handler: ~2 KB RAM
- NVS Storage: ~1 KB RAM (for preferences)
- Web Server: ~8 KB RAM (when active)
- **Total WiFi Impact:** ~12 KB RAM (non-blocking operation)

### Connection Speed
- Portal access: ~500ms from boot button press
- Network connection: 3-10 seconds typical
- Reconnection: 1-5 seconds

### Power Consumption
- Connected (minimal traffic): ~80 mA
- Portal active: ~100 mA
- Disconnected (searching): ~90 mA

---

## Next Steps

### Phase 3: Cloud Integration
- [ ] Add MQTT client for real-time data
- [ ] Add HTTP endpoints for data submission
- [ ] Implement OTA (Over-The-Air) firmware updates

### Phase 4: Remote Configuration
- [ ] Fetch configuration from Firebase
- [ ] Update thresholds over WiFi
- [ ] Remote enable/disable features

### Phase 5: Mobile Dashboard
- [ ] Real-time data streaming
- [ ] Alert notifications
- [ ] Historical data logging

---

## References

- [ESP32 WiFi API Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html)
- [ESP32 Preferences (NVS) Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/libraries.html#preferences)
- [WebServer Library Documentation](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)

---

**Last Updated:** January 23, 2026  
**WiFi Implementation Status:** ✅ Phase 2 Complete  
**Next Phase:** Firebase Integration (Phase 3)
