// --- All includes must be at the very top ---
#include "actuators.h"
#include "config.h"
#include "lidar_sensor.h"
#include "light_sensor.h"
#include "mpu6050_sensor.h"
#include "ema_filter.h"
#include <Arduino.h>
#include <FirebaseESP32.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <Preferences.h>
#include "ema_filter.h"

// Declare EMAFilter objects globally
EMAFilter lightFilter(EMA_ALPHA_LIGHT);
EMAFilter lidarFilter(EMA_ALPHA_LIDAR);
EMAFilter accelXFilter(EMA_ALPHA_MPU);
EMAFilter tiltSideFilter(EMA_ALPHA_MPU);
EMAFilter tiltFBFilter(EMA_ALPHA_MPU);

// --- Globals ---
struct SensorData {
  float lumensRaw;
  float distanceRaw;
  float accelXRaw, tiltSideRaw, tiltFBRaw;
};
static SensorData sharedData;
static SemaphoreHandle_t dataMutex;
volatile bool pauseUploads = false;
WiFiManager wm;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
WiFiUDP ntpUDP;
Preferences preferences;
// Define BUTTON_PIN if not in config.h
#ifndef BUTTON_PIN
#define BUTTON_PIN 0 // ESP32 boot button (GPIO0)
#endif

// --- Shared actuator/warning logic ---
static bool getFogLightState(float lumens) { return lumens < LIGHT_THRESHOLD; }
static bool getWarningLightState(float distance, float tiltSide, float tiltFB) {
  return (distance > 0 && distance < DIST_THRESHOLD) ||
         (fabs(tiltSide) > TILT_SIDE_THRESHOLD ||
          fabs(tiltFB) > TILT_FB_THRESHOLD);
}
static bool getBuzzerState(float distance, float tiltSide, float tiltFB) {
  return getWarningLightState(distance, tiltSide, tiltFB);
}
static String getWarningMessage(float distance, float tiltSide, float tiltFB) {
  bool warnLidar = (distance > 0 && distance < DIST_THRESHOLD);
  bool warnMpu = (fabs(tiltSide) > TILT_SIDE_THRESHOLD ||
                  fabs(tiltFB) > TILT_FB_THRESHOLD);
  if (warnLidar && warnMpu)
    return String("Lidar+MPU warning");
  if (warnLidar)
    return String("Lidar warning");
  if (warnMpu)
    return String("MPU warning");
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
      json.set("sensors/light", dataCopy.lumensRaw);
      json.set("sensors/lidar", dataCopy.distanceRaw);
      json.set("sensors/tilt_side", dataCopy.tiltSideRaw);
      json.set("sensors/tilt_fb", dataCopy.tiltFBRaw);
      json.set("sensors/accel_x", dataCopy.accelXRaw);
      json.set("actuators/fog_light", getFogLightState(dataCopy.lumensRaw));
      json.set("actuators/warning_light",
               getWarningLightState(dataCopy.distanceRaw,
                                    dataCopy.tiltSideRaw,
                                    dataCopy.tiltFBRaw));
      json.set("actuators/buzzer",
               getBuzzerState(dataCopy.distanceRaw, dataCopy.tiltSideRaw,
                              dataCopy.tiltFBRaw));
      json.set("warning", getWarningMessage(dataCopy.distanceRaw,
                                            dataCopy.tiltSideRaw,
                                            dataCopy.tiltFBRaw));
      json.set("timestamp/.sv", "timestamp");
      if (Firebase.pushJSON(fbdo, path.c_str(), json)) {
        Serial.println("[Core0] Data sent to Firebase successfully with "
                       "server-side timestamp field.");
      } else {
        Serial.print("[Core0] Failed to send data to Firebase: ");
        Serial.println(fbdo.errorReason());
      }
    } else {
      Serial.println("[Core0] Firebase not ready.");
    }
  }
}

// Define modifiable global variables for thresholds
float TILT_SIDE_THRESHOLD = 30.0f;
float TILT_FB_THRESHOLD = 9.0f;
float DIST_THRESHOLD = 120.0f;
float LIGHT_THRESHOLD = 1000.0f;

// Define modifiable global variables for sensor adjustments
float LIGHT_ADJUSTMENT = 0.0f;
float LIDAR_ADJUSTMENT = 0.0f;
float ACCEL_X_ADJUSTMENT = 0.0f;
float TILT_SIDE_ADJUSTMENT = 0.0f;
float TILT_FB_ADJUSTMENT = 0.0f;

void saveThresholdsToNVS() {
  preferences.begin("config", false);
  preferences.putFloat("TILT_SIDE_THRESHOLD", TILT_SIDE_THRESHOLD);
  preferences.putFloat("TILT_FB_THRESHOLD", TILT_FB_THRESHOLD);
  preferences.putFloat("DIST_THRESHOLD", DIST_THRESHOLD);
  preferences.putFloat("LIGHT_THRESHOLD", LIGHT_THRESHOLD);
  preferences.end();
}

// Function to load thresholds from NVS
void loadThresholdsFromNVS() {
  preferences.begin("config", true);
  TILT_SIDE_THRESHOLD = preferences.getFloat("TILT_SIDE_THRESHOLD", 30.0f);
  TILT_FB_THRESHOLD = preferences.getFloat("TILT_FB_THRESHOLD", 9.0f);
  DIST_THRESHOLD = preferences.getFloat("DIST_THRESHOLD", 120.0f);
  LIGHT_THRESHOLD = preferences.getFloat("LIGHT_THRESHOLD", 1000.0f);
  preferences.end();
}

// Function to update configuration dynamically from Firebase
void updateConfigFromFirebase() {
  Serial.println("Attempting to fetch configuration from Firebase...");

  if (Firebase.getJSON(fbdo, "/parameters")) {
    Serial.println("Configuration fetched successfully.");

    // Log the raw JSON fetched from Firebase
    String rawJson;
    fbdo.jsonObject().toString(rawJson, true);
    Serial.print("Raw JSON: ");
    Serial.println(rawJson);

    FirebaseJson &json = fbdo.jsonObject();
    FirebaseJsonData jsonData;

    // Update thresholds
    if (json.get(jsonData, "TILT_SIDE_THRESHOLD")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("TILT_SIDE_THRESHOLD: ");
        Serial.println((float)jsonData.floatValue);
        TILT_SIDE_THRESHOLD = (float)jsonData.floatValue;
      } else {
        Serial.println("TILT_SIDE_THRESHOLD key found but type mismatch.");
      }
    } else {
      Serial.println("TILT_SIDE_THRESHOLD key not found.");
    }

    if (json.get(jsonData, "TILT_FB_THRESHOLD")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("TILT_FB_THRESHOLD: ");
        Serial.println((float)jsonData.floatValue);
        TILT_FB_THRESHOLD = (float)jsonData.floatValue;
      } else {
        Serial.println("TILT_FB_THRESHOLD key found but type mismatch.");
      }
    } else {
      Serial.println("TILT_FB_THRESHOLD key not found.");
    }

    if (json.get(jsonData, "DIST_THRESHOLD")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("DIST_THRESHOLD: ");
        Serial.println((float)jsonData.floatValue);
        DIST_THRESHOLD = (float)jsonData.floatValue;
      } else {
        Serial.println("DIST_THRESHOLD key found but type mismatch.");
      }
    } else {
      Serial.println("DIST_THRESHOLD key not found.");
    }

    if (json.get(jsonData, "LIGHT_THRESHOLD")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("LIGHT_THRESHOLD: ");
        Serial.println((float)jsonData.floatValue);
        LIGHT_THRESHOLD = (float)jsonData.floatValue;
      } else {
        Serial.println("LIGHT_THRESHOLD key found but type mismatch.");
      }
    } else {
      Serial.println("LIGHT_THRESHOLD key not found.");
    }

    // Update EMA sensitivity
    if (json.get(jsonData, "EMA_ALPHA_LIGHT")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("EMA_ALPHA_LIGHT: ");
        Serial.println((float)jsonData.floatValue);
        lightFilter.setAlpha((float)jsonData.floatValue);
      } else {
        Serial.println("EMA_ALPHA_LIGHT key found but type mismatch.");
      }
    } else {
      Serial.println("EMA_ALPHA_LIGHT key not found.");
    }

    if (json.get(jsonData, "EMA_ALPHA_LIDAR")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("EMA_ALPHA_LIDAR: ");
        Serial.println((float)jsonData.floatValue);
        lidarFilter.setAlpha((float)jsonData.floatValue);
      } else {
        Serial.println("EMA_ALPHA_LIDAR key found but type mismatch.");
      }
    } else {
      Serial.println("EMA_ALPHA_LIDAR key not found.");
    }

    if (json.get(jsonData, "EMA_ALPHA_MPU")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("EMA_ALPHA_MPU: ");
        Serial.println((float)jsonData.floatValue);
        accelXFilter.setAlpha((float)jsonData.floatValue);
        tiltSideFilter.setAlpha((float)jsonData.floatValue);
        tiltFBFilter.setAlpha((float)jsonData.floatValue);
      } else {
        Serial.println("EMA_ALPHA_MPU key found but type mismatch.");
      }
    } else {
      Serial.println("EMA_ALPHA_MPU key not found.");
    }

    // Update sensor adjustments
    if (json.get(jsonData, "LIGHT_ADJUSTMENT")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("LIGHT_ADJUSTMENT: ");
        Serial.println((float)jsonData.floatValue);
        LIGHT_ADJUSTMENT = (float)jsonData.floatValue;
      } else {
        Serial.println("LIGHT_ADJUSTMENT key found but type mismatch.");
      }
    } else {
      Serial.println("LIGHT_ADJUSTMENT key not found.");
    }

    if (json.get(jsonData, "LIDAR_ADJUSTMENT")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("LIDAR_ADJUSTMENT: ");
        Serial.println((float)jsonData.floatValue);
        LIDAR_ADJUSTMENT = (float)jsonData.floatValue;
      } else {
        Serial.println("LIDAR_ADJUSTMENT key found but type mismatch.");
      }
    } else {
      Serial.println("LIDAR_ADJUSTMENT key not found.");
    }

    if (json.get(jsonData, "ACCEL_X_ADJUSTMENT")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("ACCEL_X_ADJUSTMENT: ");
        Serial.println((float)jsonData.floatValue);
        ACCEL_X_ADJUSTMENT = (float)jsonData.floatValue;
      } else {
        Serial.println("ACCEL_X_ADJUSTMENT key found but type mismatch.");
      }
    } else {
      Serial.println("ACCEL_X_ADJUSTMENT key not found.");
    }

    if (json.get(jsonData, "TILT_SIDE_ADJUSTMENT")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("TILT_SIDE_ADJUSTMENT: ");
        Serial.println((float)jsonData.floatValue);
        TILT_SIDE_ADJUSTMENT = (float)jsonData.floatValue;
      } else {
        Serial.println("TILT_SIDE_ADJUSTMENT key found but type mismatch.");
      }
    } else {
      Serial.println("TILT_SIDE_ADJUSTMENT key not found.");
    }

    if (json.get(jsonData, "TILT_FB_ADJUSTMENT")) {
      if (jsonData.type == "float" || jsonData.type == "int") {
        Serial.print("TILT_FB_ADJUSTMENT: ");
        Serial.println((float)jsonData.floatValue);
        TILT_FB_ADJUSTMENT = (float)jsonData.floatValue;
      } else {
        Serial.println("TILT_FB_ADJUSTMENT key found but type mismatch.");
      }
    } else {
      Serial.println("TILT_FB_ADJUSTMENT key not found.");
    }

    // Save updated thresholds and adjustments to NVS
    saveThresholdsToNVS();
    Serial.println("Configuration updated and saved to NVS.");
  } else {
    Serial.print("Failed to fetch configuration: ");
    Serial.println(fbdo.errorReason());
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (DEBUG_MODE)
    Serial.println("Wheelio System Booting...");

  // Turn off all actuators and sensors initially
  setFogLight(false);
  setWarningLight(false);
  setBuzzer(false);

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

  // Initialize sensors and actuators only after WiFi setup
  lightSensorInit();
  mpu6050Init();
  lidarInit();
  actuatorsInit();

  // Create mutex and start Firebase upload task (after all init is done)
  dataMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(firebaseTask, "firebaseTask", 8192, NULL, 1, NULL, 0);

  // Load thresholds from NVS on boot
  loadThresholdsFromNVS();

  // Fetch initial configuration from Firebase
  updateConfigFromFirebase();

  // Set up a periodic task to fetch updates from Firebase
  xTaskCreatePinnedToCore([](void *pvParameters) {
    for (;;) {
      updateConfigFromFirebase();
      vTaskDelay(60000 / portTICK_PERIOD_MS); // Fetch updates every 60 seconds
    }
  }, "ConfigUpdater", 8192, NULL, 1, NULL, 0);
}

void loop() {
  // Declare the missing variable
  static unsigned long lastPrintTime = 0;

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

  // Sensor/control logic every 100ms
  static unsigned long lastSensorUpdate = 0;
  if (currentMillis - lastSensorUpdate >= 100) {
    lastSensorUpdate = currentMillis;

    // Read raw sensors
    float lumensRaw = readLightLevel();
    int distanceRaw = readLidarDistance();
    MpuData mpuRaw = readMpuData();

    // Apply EMA filters and adjustments
    sharedData.lumensRaw = lightFilter.update(lumensRaw) + LIGHT_ADJUSTMENT;
    sharedData.distanceRaw = lidarFilter.update(distanceRaw) + LIDAR_ADJUSTMENT;
    sharedData.accelXRaw = accelXFilter.update(mpuRaw.accelX) + ACCEL_X_ADJUSTMENT;
    sharedData.tiltSideRaw = tiltSideFilter.update(mpuRaw.tiltSide) + TILT_SIDE_ADJUSTMENT;
    sharedData.tiltFBRaw = tiltFBFilter.update(mpuRaw.tiltFB) + TILT_FB_ADJUSTMENT;

    // Use smoothed and adjusted values for logic
    bool fogOn = sharedData.lumensRaw < LIGHT_THRESHOLD;
    setFogLight(fogOn);

    bool warnLidar = (sharedData.distanceRaw > 0 && sharedData.distanceRaw < DIST_THRESHOLD);
    bool warnMpu = (fabs(sharedData.tiltSideRaw) > TILT_SIDE_THRESHOLD ||
                    fabs(sharedData.tiltFBRaw) > TILT_FB_THRESHOLD);
    bool warning = warnLidar || warnMpu;

    setWarningLight(warning);
    setBuzzer(warning);

    // if (DEBUG_MODE) {
    //   Serial.print("Lumens (adjusted): ");
    //   Serial.print(sharedData.lumensRaw);
    //   Serial.print(" | Distance (adjusted): ");
    //   Serial.print(sharedData.distanceRaw);
    //   Serial.print(" | Tilt Side (adjusted): ");
    //   Serial.print(sharedData.tiltSideRaw);
    //   Serial.print(" | Tilt FB (adjusted): ");
    //   Serial.println(sharedData.tiltFBRaw);
    // }

    // Update shared data for Firebase task
    if (xSemaphoreTake(dataMutex, (TickType_t)10) == pdTRUE) {
      sharedData = sharedData;
      xSemaphoreGive(dataMutex);
    }
  }

  // Non-blocking timing for serial print every second
  if (currentMillis - lastPrintTime >= 1000) {
    lastPrintTime = currentMillis;

    Serial.println("Current Configuration:");
    Serial.print("TILT_SIDE_THRESHOLD: ");
    Serial.println(TILT_SIDE_THRESHOLD);
    Serial.print("TILT_FB_THRESHOLD: ");
    Serial.println(TILT_FB_THRESHOLD);
    Serial.print("DIST_THRESHOLD: ");
    Serial.println(DIST_THRESHOLD);
    Serial.print("LIGHT_THRESHOLD: ");
    Serial.println(LIGHT_THRESHOLD);

    Serial.print("EMA_ALPHA_LIGHT: ");
    Serial.println(lightFilter.getValue());
    Serial.print("EMA_ALPHA_LIDAR: ");
    Serial.println(lidarFilter.getValue());
    Serial.print("EMA_ALPHA_MPU: ");
    Serial.println(accelXFilter.getValue());

    Serial.print("LIGHT_ADJUSTMENT: ");
    Serial.println(LIGHT_ADJUSTMENT);
    Serial.print("LIDAR_ADJUSTMENT: ");
    Serial.println(LIDAR_ADJUSTMENT);
    Serial.print("ACCEL_X_ADJUSTMENT: ");
    Serial.println(ACCEL_X_ADJUSTMENT);
    Serial.print("TILT_SIDE_ADJUSTMENT: ");
    Serial.println(TILT_SIDE_ADJUSTMENT);
    Serial.print("TILT_FB_ADJUSTMENT: ");
    Serial.println(TILT_FB_ADJUSTMENT);

    // Print sensor values
    Serial.println("Current Sensor Values:");
    Serial.print("Lumens (Raw): ");
    Serial.println(sharedData.lumensRaw);
    Serial.print("Distance (Raw): ");
    Serial.println(sharedData.distanceRaw);
    Serial.print("Accel X (Raw): ");
    Serial.println(sharedData.accelXRaw);
    Serial.print("Tilt Side (Raw): ");
    Serial.println(sharedData.tiltSideRaw);
    Serial.print("Tilt FB (Raw): ");
    Serial.println(sharedData.tiltFBRaw);
  }

  // Firebase upload is handled by the FreeRTOS task on Core 0
}
