#ifndef FIREBASE_HANDLER_H
#define FIREBASE_HANDLER_H

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Sensor data structure for telemetry
struct SensorData {
    float distance_cm;
    float tilt_forward_back;
    float tilt_side;
    int light_level;
    float acceleration;
    bool fog_light_active;
    bool warning_active;
    int rssi;
    String timestamp;
};

// Calibration command structure from Firebase
struct CalibrationCommand {
    float distance_offset;
    float tilt_fb_zero;
    float tilt_side_zero;
    bool apply;
};

class FirebaseHandler {
public:
    FirebaseHandler();
    ~FirebaseHandler();
    
    // Initialize Firebase connection (HTTP REST API with DB Secret)
    void begin(const String& databaseURL, const String& databaseSecret);
    
    // Call in main loop (lightweight for HTTP mode)
    void update();
    
    // Send sensor telemetry data to Firebase via HTTP PUT
    bool sendSensorData(const SensorData& data, const String& deviceID);
    
    // Check for calibration commands from Firebase (HTTP GET)
    bool checkCalibrationCommand(const String& deviceID, CalibrationCommand& cmd);
    
    // Update device status via HTTP PATCH
    bool updateStatus(const String& deviceID, const String& status);
    
    // Check if Firebase is configured
    bool isConnected() const;
    
    // Get last error message
    String getLastError() const;

private:
    HTTPClient _http;
    WiFiClientSecure _wifiClient;
    
    String _databaseURL;
    String _databaseSecret;
    bool _initialized;
    String _lastError;
    unsigned long _lastCommandCheckTime;
    unsigned long _lastTelemetrySendTime;
    
    // Helper to build authenticated Firebase URL
    String buildFirebaseURL(const String& path);
    
    // Helper to perform HTTP PUT request
    bool httpPut(const String& path, const String& jsonData);
    
    // Helper to perform HTTP POST request (Push)
    bool httpPost(const String& path, const String& jsonData);
    
    // Helper to perform HTTP GET request
    bool httpGet(const String& path, String& response);
    
    static const unsigned long COMMAND_CHECK_INTERVAL_MS = 5000;    // Check every 5 seconds
    static const unsigned long TELEMETRY_SEND_INTERVAL_MS = 10000;  // Send every 10 seconds
};

#endif // FIREBASE_HANDLER_H
