#include "FirebaseHandler.h"
#include "config.h"

#define FIREBASE_SENSOR_PATH "/sensor_readings_v4"

FirebaseHandler::FirebaseHandler() 
    : _initialized(false),
      _lastCommandCheckTime(0),
      _lastTelemetrySendTime(0) {
    // Configure SSL client to skip certificate validation (for development)
    _wifiClient.setInsecure();
}

FirebaseHandler::~FirebaseHandler() {
    _http.end();
}

void FirebaseHandler::begin(const String& databaseURL, const String& databaseSecret) {
    Serial.println("\n[Firebase] Initializing HTTP REST API Handler (Secret Auth)...");
    
    _databaseURL = databaseURL;
    _databaseSecret = databaseSecret;
    _initialized = true;
    
    Serial.println("[Firebase] ✓ HTTP REST API configured");
    Serial.print("[Firebase] Database: ");
    Serial.println(_databaseURL);
}

void FirebaseHandler::update() {
    // No background tasks needed for simple HTTP REST
}

String FirebaseHandler::buildFirebaseURL(const String& path) {
    // Firebase REST API URL format for Legacy Tokens:
    // https://<DATABASE>.firebaseio.com/<path>.json?auth=<SECRET>
    String url = _databaseURL;
    
    // Remove trailing slash if present
    if (url.endsWith("/")) {
        url = url.substring(0, url.length() - 1);
    }
    
    // Add path
    url += path;
    
    // Add .json extension
    url += ".json";
    
    // Add authentication via query parameter (Legacy Token / DB Secret)
    url += "?auth=";
    url += _databaseSecret;
    
    return url;
}

bool FirebaseHandler::httpPut(const String& path, const String& jsonData) {
    if (!_initialized || WiFi.status() != WL_CONNECTED) {
        _lastError = "Not initialized or WiFi disconnected";
        return false;
    }
    
    String url = buildFirebaseURL(path);
    
    _http.begin(_wifiClient, url);
    _http.addHeader("Content-Type", "application/json");
    
    int httpCode = _http.PUT(jsonData);
    
    bool success = false;
    if (httpCode == HTTP_CODE_OK || httpCode == 200) {
        success = true;
        _lastError = "";
    } else {
        String response = _http.getString();
        _lastError = "HTTP PUT failed: " + String(httpCode);
        Serial.printf("[Firebase] PUT failed: %d - %s\n", httpCode, _http.errorToString(httpCode).c_str());
        if (response.length() > 0) {
            Serial.println("[Firebase] Error response: " + response);
        }
    }
    
    _http.end();
    return success;
}

bool FirebaseHandler::httpPost(const String& path, const String& jsonData) {
    if (!_initialized || WiFi.status() != WL_CONNECTED) {
        _lastError = "Not initialized or WiFi disconnected";
        return false;
    }
    
    String url = buildFirebaseURL(path);
    
    _http.begin(_wifiClient, url);
    _http.addHeader("Content-Type", "application/json");
    
    // POST in Firebase REST API creates a new child node with a unique name (Push)
    int httpCode = _http.POST(jsonData);
    
    bool success = false;
    if (httpCode == HTTP_CODE_OK || httpCode == 200) {
        success = true;
        _lastError = "";
    } else {
        String response = _http.getString();
        _lastError = "HTTP POST failed: " + String(httpCode);
        Serial.printf("[Firebase] POST failed: %d - %s\n", httpCode, _http.errorToString(httpCode).c_str());
        if (response.length() > 0) {
            Serial.println("[Firebase] Error response: " + response);
        }
    }
    
    _http.end();
    return success;
}

bool FirebaseHandler::httpGet(const String& path, String& response) {
    if (!_initialized || WiFi.status() != WL_CONNECTED) {
        _lastError = "Not initialized or WiFi disconnected";
        return false;
    }
    
    String url = buildFirebaseURL(path);
    
    _http.begin(_wifiClient, url);
    
    int httpCode = _http.GET();
    
    bool success = false;
    if (httpCode == HTTP_CODE_OK || httpCode == 200) {
        response = _http.getString();
        success = true;
        _lastError = "";
    } else {
        String errorResp = _http.getString();
        _lastError = "HTTP GET failed: " + String(httpCode);
        Serial.printf("[Firebase] GET failed: %d - %s\n", httpCode, _http.errorToString(httpCode).c_str());
        if (errorResp.length() > 0) {
            Serial.println("[Firebase] Error response: " + errorResp);
        }
    }
    
    _http.end();
    return success;
}

bool FirebaseHandler::sendSensorData(const SensorData& data, const String& deviceID) {
    if (!_initialized) {
        Serial.println("[Firebase] Cannot send data - not initialized");
        return false;
    }
    
    // Paths: 
    // 1. /sensor_readings_v4/{deviceID}/latest  -> Overwritten (for live status)
    // 2. /sensor_readings_v4/{deviceID}/history -> Appended (for historical data)
    String latestPath = String(FIREBASE_SENSOR_PATH) + "/" + deviceID + "/latest";
    String historyPath = String(FIREBASE_SENSOR_PATH) + "/" + deviceID + "/history";
    
    // Create JSON using ArduinoJson
    StaticJsonDocument<512> doc;
    doc["distance_cm"] = round(data.distance_cm * 100) / 100.0;
    doc["tilt_fb"] = round(data.tilt_forward_back * 100) / 100.0;
    doc["tilt_side"] = round(data.tilt_side * 100) / 100.0;
    doc["light_level"] = data.light_level;
    doc["acceleration"] = round(data.acceleration * 100) / 100.0;
    doc["fog_active"] = data.fog_light_active;
    doc["warning_active"] = data.warning_active;
    doc["rssi"] = data.rssi;
    doc["timestamp"] = data.timestamp;
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    
    // 1. Update latest (PUT)
    bool latestSuccess = httpPut(latestPath, jsonStr);
    
    // 2. Append to history (POST)
    bool historySuccess = httpPost(historyPath, jsonStr);
    
    if (latestSuccess && historySuccess) {
        Serial.printf("[Firebase] ✓ Success: Telemetry saved to Latest & History\n");
        Serial.printf("[Firebase] Data: L=%d D=%.0f TFB=%.1f TS=%.1f A=%.2f RSSI=%d\n",
                      data.light_level, data.distance_cm, data.tilt_forward_back, 
                      data.tilt_side, data.acceleration, data.rssi);
    } else {
        Serial.println("[Firebase] ✗ Partial or total failure sending telemetry");
    }
    
    _lastTelemetrySendTime = millis();
    return latestSuccess && historySuccess;
}

bool FirebaseHandler::checkCalibrationCommand(const String& deviceID, CalibrationCommand& cmd) {
    if (!_initialized) {
        return false;
    }
    
    unsigned long now = millis();
    if (now - _lastCommandCheckTime < COMMAND_CHECK_INTERVAL_MS) {
        return false; 
    }
    _lastCommandCheckTime = now;
    
    String path = "/commands/" + deviceID + "/calibration";
    
    String response;
    if (!httpGet(path, response)) {
        return false;
    }
    
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (error || response == "null" || doc.isNull()) {
        return false;
    }
    
    cmd.distance_offset = doc["distance_offset"] | 0.0f;
    cmd.tilt_fb_zero = doc["tilt_fb_zero"] | 0.0f;
    cmd.tilt_side_zero = doc["tilt_side_zero"] | 0.0f;
    cmd.apply = doc["apply"] | false;
    
    Serial.println("[Firebase] ✓ Calibration command received");
    return true;
}

bool FirebaseHandler::updateStatus(const String& deviceID, const String& status) {
    if (!_initialized) {
        return false;
    }
    
    String path = "/devices/" + deviceID + "/status";
    
    StaticJsonDocument<128> doc;
    doc["status"] = status;
    doc["timestamp"] = millis() / 1000;
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    
    return httpPut(path, jsonStr);
}

bool FirebaseHandler::isConnected() const {
    return _initialized && (WiFi.status() == WL_CONNECTED);
}

String FirebaseHandler::getLastError() const {
    return _lastError;
}
