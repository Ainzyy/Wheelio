

// --- All includes must be at the very top ---
#include <Arduino.h>
#include "config.h"
#include "actuators.h"
#include "lidar_sensor.h"
#include "light_sensor.h"
#include "mpu6050_sensor.h"
#include <FirebaseESP32.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

// Smoothing filter constant
#ifndef SMOOTHING_ALPHA
#define SMOOTHING_ALPHA 0.45f // 0.0 = no smoothing, 1.0 = instant
#endif

// --- Globals ---
struct SensorData {
  float lumensSmooth;
  float distanceSmooth;
  float accelXSmooth, tiltSideSmooth, tiltFBSmooth;
};
static SensorData sharedData;
static SemaphoreHandle_t dataMutex;
volatile bool pauseUploads = false;
WiFiManager wm;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiUDP ntpUDP;
// Define BUTTON_PIN if not in config.h
#ifndef BUTTON_PIN
#define BUTTON_PIN 0 // ESP32 boot button (GPIO0)
#endif


// --- Shared actuator/warning logic ---
static bool getFogLightState(float lumens) {
  return lumens < LIGHT_THRESHOLD;
}
static bool getWarningLightState(float distance, float tiltSide, float tiltFB) {
  return (distance > 0 && distance < DIST_THRESHOLD) ||
         (fabs(tiltSide) > TILT_SIDE_THRESHOLD || fabs(tiltFB) > TILT_FB_THRESHOLD);
}
static bool getBuzzerState(float distance, float tiltSide, float tiltFB) {
  return getWarningLightState(distance, tiltSide, tiltFB);
}
static String getWarningMessage(float distance, float tiltSide, float tiltFB) {
  bool warnLidar = (distance > 0 && distance < DIST_THRESHOLD);
  bool warnMpu = (fabs(tiltSide) > TILT_SIDE_THRESHOLD || fabs(tiltFB) > TILT_FB_THRESHOLD);
  if (warnLidar && warnMpu) return String("Lidar+MPU warning");
  if (warnLidar) return String("Lidar warning");
  if (warnMpu) return String("MPU warning");
  return String("None");
}

// --- Firebase upload task (runs on Core 0) ---
void firebaseTask(void *pvParameters) {
  for (;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1s interval
    SensorData dataCopy;
    if (xSemaphoreTake(dataMutex, (TickType_t)10) == pdTRUE) {
      dataCopy = sharedData;
      xSemaphoreGive(dataMutex);
    } else {
      continue; // skip this cycle if can't get data
    }

    if (pauseUploads) {
      // Paused: skip upload
      continue;
    }
    if (Firebase.ready()) {
      String path = "/sensor_readings_test2";
      FirebaseJson json;
      json.set("sensors/light", dataCopy.lumensSmooth);
      json.set("sensors/lidar", dataCopy.distanceSmooth);
      json.set("sensors/tilt_side", dataCopy.tiltSideSmooth);
      json.set("sensors/tilt_fb", dataCopy.tiltFBSmooth);
      json.set("sensors/accel_x", dataCopy.accelXSmooth);
      json.set("actuators/fog_light", getFogLightState(dataCopy.lumensSmooth));
      json.set("actuators/warning_light", getWarningLightState(dataCopy.distanceSmooth, dataCopy.tiltSideSmooth, dataCopy.tiltFBSmooth));
      json.set("actuators/buzzer", getBuzzerState(dataCopy.distanceSmooth, dataCopy.tiltSideSmooth, dataCopy.tiltFBSmooth));
      json.set("warning", getWarningMessage(dataCopy.distanceSmooth, dataCopy.tiltSideSmooth, dataCopy.tiltFBSmooth));
      json.set("timestamp/.sv", "timestamp");
      if (Firebase.pushJSON(fbdo, path.c_str(), json)) {
        Serial.println("[Core0] Data sent to Firebase successfully with server-side timestamp field.");
      } else {
        Serial.print("[Core0] Failed to send data to Firebase: ");
        Serial.println(fbdo.errorReason());
      }
    } else {
      Serial.println("[Core0] Firebase not ready.");
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (DEBUG_MODE)
    Serial.println("Wheelio System Booting...");

  // Use WiFiManager to auto-connect to known WiFi or open config portal if
  // needed
  wm.setConfigPortalTimeout(180);
  if (!wm.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD)) {
    Serial.println("Failed to connect to WiFi or config portal timed out.");
  } else {
    Serial.println("WiFi connected.");
  }

  if (DEBUG_MODE)
    Serial.print("WiFi connected: ");
  if (DEBUG_MODE)
    Serial.println(WiFi.localIP());


  // Firebase (modern auth)
  config.api_key = API_KEY;
  config.database_url = FIREBASE_HOST;
  auth.user.email = EMAIL;
  auth.user.password = PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  lightSensorInit();
  mpu6050Init();
  lidarInit();
  actuatorsInit();

  // Create mutex and start Firebase upload task (after all init is done)
  dataMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(firebaseTask, "firebaseTask", 8192, NULL, 1, NULL, 0);
}

void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long currentMillis = millis();

  // Check if the boot button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!pauseUploads) {
      pauseUploads = true;
      Serial.println("Uploads paused (boot button pressed)");
    }
    Serial.println("Boot button pressed. Opening WiFi configuration portal...");
    wm.setConfigPortalTimeout(180);
    if (!wm.startConfigPortal(WIFI_AP_NAME, WIFI_AP_PASSWORD)) {
      Serial.println("Failed to connect to WiFi through configuration portal.");
    } else {
      Serial.println("WiFi connected through configuration portal.");
    }
  } else {
    if (pauseUploads) {
      pauseUploads = false;
      Serial.println("Uploads resumed (boot button released)");
    }
  }

  // --- Smoothing filter state (static persists across calls) ---
  static float lumensSmooth = 0;
  static float distanceSmooth = 0;
  static float accelXSmooth = 0, tiltSideSmooth = 0, tiltFBSmooth = 0;
  static bool firstRun = true;

  // Sensor/control logic every 100ms
  static unsigned long lastSensorUpdate = 0;
  if (currentMillis - lastSensorUpdate >= 100) {
    lastSensorUpdate = currentMillis;

    // Read raw sensors
    float lumensRaw = readLightLevel();
    int distanceRaw = readLidarDistance();
    MpuData mpuRaw = readMpuData();

    // Initialize smoothing on first run
    if (firstRun) {
      lumensSmooth = lumensRaw;
      distanceSmooth = distanceRaw;
      accelXSmooth = mpuRaw.accelX;
      tiltSideSmooth = mpuRaw.tiltSide;
      tiltFBSmooth = mpuRaw.tiltFB;
      firstRun = false;
    } else {
      // Apply smoothing filters
      lumensSmooth =
          SMOOTHING_ALPHA * lumensRaw + (1.0f - SMOOTHING_ALPHA) * lumensSmooth;
      distanceSmooth = SMOOTHING_ALPHA * distanceRaw +
                       (1.0f - SMOOTHING_ALPHA) * distanceSmooth;
      accelXSmooth = SMOOTHING_ALPHA * mpuRaw.accelX +
                     (1.0f - SMOOTHING_ALPHA) * accelXSmooth;
      tiltSideSmooth = SMOOTHING_ALPHA * mpuRaw.tiltSide +
                       (1.0f - SMOOTHING_ALPHA) * tiltSideSmooth ;
      tiltFBSmooth = SMOOTHING_ALPHA * mpuRaw.tiltFB +
                     (1.0f - SMOOTHING_ALPHA) * tiltFBSmooth;
    }

    tiltSideSmooth += 1.4;
    tiltFBSmooth -= 6.5;

    // Use smoothed values for logic
    bool fogOn = lumensSmooth < LIGHT_THRESHOLD;
    setFogLight(fogOn);

    // Shared warning mechanism for Lidar and MPU
    bool warnLidar = (distanceSmooth > 0 && distanceSmooth < DIST_THRESHOLD);
    bool warnMpu = (fabs(tiltSideSmooth) > TILT_SIDE_THRESHOLD ||
                    fabs(tiltFBSmooth) > TILT_FB_THRESHOLD);
    bool warning = warnLidar || warnMpu;

    // Active low relays
    setWarningLight(warning);
    setBuzzer(warning);

    // Debugging output
    if (DEBUG_MODE) {
      Serial.print("Lumens: ");
      Serial.print(lumensSmooth);
      Serial.print(" | Distance: ");
      Serial.print(distanceSmooth);
      Serial.print(" | Tilt Side: ");
      Serial.print(tiltSideSmooth);
      Serial.print(" | Tilt FB: ");
      Serial.print(tiltFBSmooth);
      Serial.print(" | Fog Light: ");
      Serial.print(fogOn ? "ON" : "OFF");
      Serial.print(" | Warning: ");
      Serial.println(warning ? "ON" : "OFF");
    }

    // Update shared data for Firebase task
    if (xSemaphoreTake(dataMutex, (TickType_t)10) == pdTRUE) {
      sharedData.lumensSmooth = lumensSmooth;
      sharedData.distanceSmooth = distanceSmooth;
      sharedData.accelXSmooth = accelXSmooth;
      sharedData.tiltSideSmooth = tiltSideSmooth;
      sharedData.tiltFBSmooth = tiltFBSmooth;
      xSemaphoreGive(dataMutex);
    }
  }

  // Firebase upload is handled by the FreeRTOS task on Core 0
}
