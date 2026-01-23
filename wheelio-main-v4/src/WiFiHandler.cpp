#include "WiFiHandler.h"
#include "config.h"

WiFiHandler::WiFiHandler() 
    : _status(WIFI_STATUS_DISCONNECTED),
      _lastReconnectAttempt(0),
      _bootButtonPressStart(0),
      _bootButtonWasPressed(false) {
}

void WiFiHandler::begin() {
    Serial.println("\n[WiFi] Initializing WiFi Handler with tzapu/WiFiManager...");
    delay(500);  // Give Serial time to stabilize
    
    // Setup boot button as input
    pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);
    
    // Debug: Check boot button state at startup
    Serial.print("[WiFi] Boot Button (GPIO ");
    Serial.print(PIN_BOOT_BUTTON);
    Serial.print(") state: ");
    Serial.println(digitalRead(PIN_BOOT_BUTTON) == LOW ? "PRESSED" : "RELEASED");
    Serial.println("[WiFi] -> Press boot button to open WiFi configuration portal");
    
    // Configure WiFiManager settings
    _wifiManager.setConnectTimeout(WIFI_PORTAL_TIMEOUT_SEC);
    _wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_SEC);
    _wifiManager.setHostname(WIFI_DEVICE_NAME);
    
    // Check if WiFiManager has saved credentials (internal check, not WiFi.SSID())
    bool hasWiFiSaved = _wifiManager.getWiFiIsSaved();
    Serial.print("[WiFi] Checking for saved credentials... ");
    if (hasWiFiSaved) {
        Serial.println("YES - attempting to connect");
        Serial.print("[WiFi] Will connect to saved network for ");
        Serial.print(WIFI_PORTAL_TIMEOUT_SEC);
        Serial.println(" seconds...");
    } else {
        Serial.println("NO - opening portal for first-time WiFi configuration");
    }
    
    // Set AP name for configuration portal
    String apName = String(WIFI_DEVICE_NAME);
    
    // autoConnect() will:
    // 1. Try to connect to saved network for WIFI_PORTAL_TIMEOUT_SEC seconds
    // 2. If it fails or no saved credentials, start config portal
    // 3. Return true if connected, false if timed out or portal exited
    Serial.println("[WiFi] Calling autoConnect()...");
    if (!_wifiManager.autoConnect(apName.c_str())) {
        Serial.println("[WiFi] Failed to connect to saved network");
        Serial.println("[WiFi] Config portal should be running now");
        Serial.println("[WiFi] If no portal appears, press BOOT button to trigger it");
        _status = WIFI_STATUS_DISCONNECTED;
    } else {
        Serial.println("[WiFi] Successfully connected to WiFi!");
        Serial.print("[WiFi] Connected to SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("[WiFi] Signal strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        _status = WIFI_STATUS_CONNECTED;
    }
    
    Serial.println("[WiFi] WiFi initialization complete!");
    delay(100);
}

void WiFiHandler::update() {
    // Process WiFiManager portal if running (non-blocking mode)
    // This handles the configuration portal web interface
    _wifiManager.process();
    
    // Check boot button for portal trigger
    checkBootButton();
    
    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
        if (_status != WIFI_STATUS_CONNECTED) {
            _status = WIFI_STATUS_CONNECTED;
            Serial.println("[WiFi] Connected!");
            Serial.print("[WiFi] IP: ");
            Serial.println(WiFi.localIP());
        }
        _lastReconnectAttempt = millis();  // Reset attempt timer
    } else {
        // Try to reconnect periodically
        if (millis() - _lastReconnectAttempt >= WIFI_RECONNECT_INTERVAL_MS) {
            if (_status == WIFI_STATUS_CONNECTED) {
                _status = WIFI_STATUS_CONNECTING;
                Serial.println("[WiFi] Connection lost, attempting to reconnect...");
            }
            WiFi.reconnect();
            _lastReconnectAttempt = millis();
        }
    }
}

WiFiStatus WiFiHandler::getStatus() const {
    return _status;
}

bool WiFiHandler::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiHandler::getSSID() const {
    return WiFi.SSID();
}

int WiFiHandler::getRSSI() const {
    return WiFi.RSSI();
}

void WiFiHandler::startPortal() {
    Serial.println("\n[WiFi] ========================================");
    Serial.println("[WiFi] WiFi Configuration Portal Starting");
    Serial.println("[WiFi] ========================================");
    
    _status = WIFI_STATUS_CONNECTING;
    
    // Disconnect from current WiFi to free up resources
    // But DON'T erase saved credentials
    if (isConnected()) {
        Serial.print("[WiFi] Disconnecting from: ");
        Serial.println(WiFi.SSID());
        WiFi.disconnect(false);  // false = don't erase saved credentials
        delay(500);
    }
    
    String apName = String(WIFI_DEVICE_NAME);
    
    Serial.println("[WiFi] ");
    Serial.println("[WiFi] === NEXT STEPS ===");
    Serial.println("[WiFi] 1. Look for WiFi network: Wheelio-ESP32");
    Serial.println("[WiFi] 2. Connect to that network");
    Serial.println("[WiFi] 3. Open browser and navigate to: http://192.168.4.1");
    Serial.println("[WiFi] 4. Select your desired WiFi network and enter password");
    Serial.print("[WiFi] 5. Portal will timeout in ");
    Serial.print(WIFI_PORTAL_TIMEOUT_SEC);
    Serial.println(" seconds if no action taken");
    Serial.println("[WiFi] ========================================\n");
    
    // Set non-blocking mode so main loop continues
    _wifiManager.setConfigPortalBlocking(false);
    _wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT_SEC);
    
    // Start config portal (non-blocking)
    _wifiManager.startConfigPortal(apName.c_str());
    
    Serial.println("[WiFi] Portal opened! User can now configure WiFi.");
}

bool WiFiHandler::isBootButtonPressed() const {
    // Boot button is LOW when pressed (pulled up by default)
    return digitalRead(PIN_BOOT_BUTTON) == LOW;
}

void WiFiHandler::checkBootButton() {
    // Boot button is LOW when pressed
    if (isBootButtonPressed()) {
        if (!_bootButtonWasPressed) {
            // Button just pressed - trigger portal
            _bootButtonWasPressed = true;
            
            Serial.println("\n[WiFi] *** Boot button pressed - opening portal ***");
            if (isConnected()) {
                Serial.print("[WiFi] Disconnecting from: ");
                Serial.println(WiFi.SSID());
            }
            Serial.println("[WiFi] Please connect to: Wheelio-ESP32");
            startPortal();
        }
    } else {
        // Button released
        if (_bootButtonWasPressed) {
            _bootButtonWasPressed = false;
        }
    }
}

void WiFiHandler::printStatus() const {
    Serial.println("\n======== WiFi Status ========");
    
    switch (_status) {
        case WIFI_STATUS_DISCONNECTED:
            Serial.println("Status: DISCONNECTED");
            break;
        case WIFI_STATUS_CONNECTING:
            Serial.println("Status: CONNECTING");
            break;
        case WIFI_STATUS_CONNECTED:
            Serial.println("Status: CONNECTED");
            Serial.print("SSID: ");
            Serial.println(WiFi.SSID());
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            Serial.print("MAC Address: ");
            Serial.println(WiFi.macAddress());
            Serial.print("Signal Strength (RSSI): ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
            break;
    }
    
    Serial.println("=============================\n");
}

