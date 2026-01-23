#include <Arduino.h>
#include <Wire.h>
#include <cstring>
#include <time.h>
#include "config.h"
#include "VL53L1XSensor.h"
#include "MPU6050Sensor.h"
#include "LightSensor.h"
#include "Relay.h"
#include "Buzzer.h"

#if ENABLE_WIFI
#include "WiFiHandler.h"
WiFiHandler wifiHandler;
#endif

#if ENABLE_FIREBASE
#include "FirebaseHandler.h"
FirebaseHandler firebaseHandler;
#endif

// Instantiate hardware modules
#if ENABLE_LIDAR
VL53L1XSensor lidar(FILTER_ALPHA_LIDAR);
#endif

#if ENABLE_IMU
MPU6050Sensor imu(FILTER_ALPHA_IMU);
#endif

#if ENABLE_LIGHT_SENSOR
LightSensor lightSensor(PIN_LIGHT_SENSOR);
#endif

#if ENABLE_FOG_LIGHT
Relay fogRelay(PIN_FOG_RELAY, FOG_LIGHT_ACTIVE_HIGH);
#endif

#if ENABLE_WARNING_SYSTEM
Relay warningRelay(PIN_WARNING_RELAY, WARNING_SYSTEM_ACTIVE_HIGH);
Buzzer buzzer(PIN_BUZZER);
#endif

unsigned long lastPollTime = 0;
unsigned long lastFirebaseSendTime = 0;
const unsigned long FIREBASE_SEND_INTERVAL = 10000; // Send every 10 seconds

struct {
    int light;
    uint16_t distance;
    float tiltFB;
    float tiltSide;
    float accelMag;
} lastSensorReading = {1000, 1000, 0, 0, 1.0};

void setup() {
#if ENABLE_DEBUG_SERIAL
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("\n--- Wheelio Initialization ---");
#endif

#if ENABLE_MOCK_DATA
    Serial.println("[MOCK] Running in SIMULATION MODE");
#else
    // Initialize I2C bus once for all sensors
    #if ENABLE_LIDAR || ENABLE_IMU
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    #endif
#endif

    // Initialize sensors
#if ENABLE_LIDAR && !ENABLE_MOCK_DATA
    if (lidar.begin()) {
        Serial.println("[OK] VL53L1X initialized");
    } else {
        Serial.println("[ERR] VL53L1X failed!");
    }
#endif

#if ENABLE_IMU && !ENABLE_MOCK_DATA
    if (imu.begin()) {
        Serial.println("[OK] MPU6050 initialized");
    } else {
        Serial.println("[ERR] MPU6050 failed!");
    }
#endif

#if ENABLE_LIGHT_SENSOR && !ENABLE_MOCK_DATA
    lightSensor.begin();
    Serial.println("[OK] Light sensor ready");
#endif

    // Initialize actuators
#if ENABLE_FOG_LIGHT
    fogRelay.begin();
    Serial.println("[OK] Fog Light ready");
#endif

#if ENABLE_WARNING_SYSTEM
    warningRelay.begin();
    buzzer.begin();
    Serial.println("[OK] Warning System ready");
#endif

#if ENABLE_WIFI
    wifiHandler.begin();
#endif

#if ENABLE_NTP
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    Serial.println("[OK] NTP sync started");
#endif

#if ENABLE_FIREBASE
    // Initialize Firebase HTTP REST API with simplified DB Secret authentication
    firebaseHandler.begin(FIREBASE_HOST, FIREBASE_DB_SECRET);
#endif

#if ENABLE_DEBUG_SERIAL
    Serial.println("[SETUP] Initialization complete");
#endif
}

void loop() {
    // Core 0: Sensor polling and actuator control (lightweight, high frequency)
    unsigned long currentTime = millis();

    // Poll sensors at defined interval
    if (currentTime - lastPollTime >= SENSOR_POLL_INTERVAL_MS) {
        lastPollTime = currentTime;

        // Default safe values
        int light = 1000;         
        uint16_t distance = 1000; 
        float tiltFB = 0.0f;
        float tiltSide = 0.0f;
        float accelMag = 1.0f;    

#if ENABLE_MOCK_DATA
        // Simulate data: periodic patterns
        light = 1100 + 400 * sin(currentTime / 5000.0);       
        distance = 300 + 200 * cos(currentTime / 3000.0);     
        tiltFB = 40.0 * sin(currentTime / 2000.0);
        tiltSide = 40.0 * sin(currentTime / 2500.0);          
        accelMag = 1.0 + 1.5 * abs(sin(currentTime / 4000.0)); 
#else
    #if ENABLE_LIGHT_SENSOR
        light = lightSensor.readLight();
    #endif

    #if ENABLE_LIDAR
        distance = lidar.readDistance();
    #endif

    #if ENABLE_IMU
        MpuData mpuValues = imu.readMpuData();
        tiltFB = mpuValues.tiltFB; 
        tiltSide = mpuValues.tiltSide;
        float rawMag = sqrt(pow(mpuValues.accelX, 2) + pow(mpuValues.accelY, 2) + pow(mpuValues.accelZ, 2));
        accelMag = rawMag / 9.81; 
    #endif
#endif

#if ENABLE_DEBUG_SERIAL
        Serial.print(ENABLE_MOCK_DATA ? "[M] " : "[R] ");
        Serial.print("L:"); Serial.print(light);
        Serial.print(" | D:"); Serial.print(distance);
        Serial.print(" | TFB:"); Serial.print(tiltFB, 1);
        Serial.print(" | TS:"); Serial.print(tiltSide, 1);
        Serial.print(" | A:"); Serial.print(accelMag, 2);
#endif

        // --- Actuator Logic ---

#if ENABLE_FOG_LIGHT
        // Fog light: Activate when light is dark
        if (light < LIGHT_THRESHOLD) {
            fogRelay.on();
#if ENABLE_DEBUG_SERIAL
            Serial.print(" | FOG:ON");
#endif
        } else {
            fogRelay.off();
        }
#endif

#if ENABLE_WARNING_SYSTEM
        // Warning logic:
        // - Proximity: distance < 200cm
        // - Tilt FB: forward/back > 9 degrees
        // - Tilt Side: side tilt > 30 degrees
        // - Impact: acceleration > 2G
        bool proximityWarn = (distance < DISTANCE_THRESHOLD_CM);
        bool tiltFBWarn = (abs(tiltFB) > TILT_FB_THRESHOLD);
        bool tiltSideWarn = (abs(tiltSide) > TILT_SIDE_THRESHOLD);
        bool impactWarn = (abs(accelMag - 1.0f) > (ACCEL_THRESHOLD - 1.0f)); 

        if (proximityWarn || tiltFBWarn || tiltSideWarn || impactWarn) {
            warningRelay.on();
            buzzer.on();
#if ENABLE_DEBUG_SERIAL
            Serial.print(" | WARN:[");
            if (proximityWarn) Serial.print("P");
            if (tiltFBWarn) Serial.print("FB");
            if (tiltSideWarn) Serial.print("S");
            if (impactWarn) Serial.print("I");
            Serial.print("]");
#endif
        } else {
            warningRelay.off();
            buzzer.off();
        }
#endif

#if ENABLE_DEBUG_SERIAL
        Serial.println();
#endif

        // Store latest sensor readings
        lastSensorReading.light = light;
        lastSensorReading.distance = distance;
        lastSensorReading.tiltFB = tiltFB;
        lastSensorReading.tiltSide = tiltSide;
        lastSensorReading.accelMag = accelMag;
    }
    
    // Firebase telemetry sending (every 10 seconds, on Core 0)
    #if ENABLE_FIREBASE
    if (currentTime - lastFirebaseSendTime >= FIREBASE_SEND_INTERVAL) {
        lastFirebaseSendTime = currentTime;
        
        // Update Firebase async client
        firebaseHandler.update();
        
        // Send telemetry if connected
        if (firebaseHandler.isConnected()) {
            SensorData telemetry;
            telemetry.distance_cm = lastSensorReading.distance;
            telemetry.tilt_forward_back = lastSensorReading.tiltFB;
            telemetry.tilt_side = lastSensorReading.tiltSide;
            telemetry.light_level = lastSensorReading.light;
            telemetry.acceleration = lastSensorReading.accelMag;
            telemetry.rssi = WiFi.RSSI();
            
            #if ENABLE_NTP
            struct tm timeinfo;
            char timeString[25];
            if (getLocalTime(&timeinfo)) {
                strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
                telemetry.timestamp = String(timeString);
            } else {
                telemetry.timestamp = "NTP Sync Pending";
            }
            #else
            telemetry.timestamp = String(millis());
            #endif

            #if ENABLE_FOG_LIGHT
            telemetry.fog_light_active = fogRelay.isOn();
            #else
            telemetry.fog_light_active = false;
            #endif
            #if ENABLE_WARNING_SYSTEM
            telemetry.warning_active = warningRelay.isOn();
            #else
            telemetry.warning_active = false;
            #endif
            
            firebaseHandler.sendSensorData(telemetry, DEVICE_ID);
        }
    }
    #endif
    
    // Update WiFi manager
    #if ENABLE_WIFI
    wifiHandler.update();
    #endif
}

