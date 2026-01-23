#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <Arduino.h>
#include <WiFiManager.h>

// WiFi connection status
enum WiFiStatus {
    WIFI_STATUS_DISCONNECTED,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED
};

class WiFiHandler {
public:
    WiFiHandler();
    
    // Initialize WiFi (handles both first-time setup and reconnection)
    void begin();
    
    // Check WiFi connection status and attempt reconnect if needed
    void update();
    
    // Get current WiFi status
    WiFiStatus getStatus() const;
    
    // Check if WiFi is connected
    bool isConnected() const;
    
    // Get connected SSID
    String getSSID() const;
    
    // Get signal strength (RSSI)
    int getRSSI() const;
    
    // Manually trigger WiFi portal (e.g., when boot button pressed)
    void startPortal();
    
    // Check if boot button is pressed
    bool isBootButtonPressed() const;
    
    // Print WiFi status to Serial
    void printStatus() const;

private:
    WiFiStatus _status;
    unsigned long _lastReconnectAttempt;
    unsigned long _bootButtonPressStart;
    bool _bootButtonWasPressed;
    WiFiManager _wifiManager;
    
    // Check boot button for long press
    void checkBootButton();
};

#endif // WIFI_HANDLER_H
