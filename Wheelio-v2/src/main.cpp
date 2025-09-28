#define SMOOTHING_ALPHA 0.2f // 0.0 = no smoothing, 1.0 = instant

#include <Arduino.h>
#include "config.h"
#include "light_sensor.h"
#include "mpu6050_sensor.h"
#include "lidar_sensor.h"

#include "actuators.h"
#include <WiFiManager.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFiManager and button
#define BUTTON_PIN 0 // ESP32 boot button (GPIO0)
WiFiManager wm;

// Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC, 60000);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (DEBUG_MODE)
    Serial.println("Wheelio System Booting...");

  // WiFiManager: always open config portal on boot
  if (DEBUG_MODE)
    Serial.println("Starting WiFiManager config portal on boot...");
  wm.setConfigPortalTimeout(180);
  wm.startConfigPortal(WIFI_AP_NAME, WIFI_AP_PASSWORD);
  if (DEBUG_MODE)
    Serial.print("WiFi connected: ");
  if (DEBUG_MODE)
    Serial.println(WiFi.localIP());

  // NTP
  timeClient.begin();
  while (!timeClient.update())
    timeClient.forceUpdate();

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
}

void loop()
{
  // ...existing code...

  // --- Smoothing filter state (static persists across calls) ---
  static float lumensSmooth = 0;
  static float distanceSmooth = 0;
  static float accelXSmooth = 0, tiltSideSmooth = 0, tiltFBSmooth = 0;
  static bool firstRun = true;

  // Read raw sensors
  float lumensRaw = readLightLevel();
  int distanceRaw = readLidarDistance();
  MpuData mpuRaw = readMpuData();

  // Initialize smoothing on first run
  if (firstRun)
  {
    lumensSmooth = lumensRaw;
    distanceSmooth = distanceRaw;
    accelXSmooth = mpuRaw.accelX;
    tiltSideSmooth = mpuRaw.tiltSide;
    tiltFBSmooth = mpuRaw.tiltFB;
    firstRun = false;
  }
  else
  {
    lumensSmooth = SMOOTHING_ALPHA * lumensRaw + (1.0f - SMOOTHING_ALPHA) * lumensSmooth;
    distanceSmooth = SMOOTHING_ALPHA * distanceRaw + (1.0f - SMOOTHING_ALPHA) * distanceSmooth;
    accelXSmooth = SMOOTHING_ALPHA * mpuRaw.accelX + (1.0f - SMOOTHING_ALPHA) * accelXSmooth;
    tiltSideSmooth = SMOOTHING_ALPHA * mpuRaw.tiltSide + (1.0f - SMOOTHING_ALPHA) * tiltSideSmooth;
    tiltFBSmooth = SMOOTHING_ALPHA * mpuRaw.tiltFB + (1.0f - SMOOTHING_ALPHA) * tiltFBSmooth;
  }

  // Use smoothed values for logic
  bool warnLidar = (distanceSmooth > 0 && distanceSmooth < DIST_THRESHOLD);
  bool warnAccel = fabs(accelXSmooth) > ACCEL_THRESHOLD;
  bool warnTiltSide = fabs(tiltSideSmooth) > TILT_SIDE_THRESHOLD;
  bool warnTiltFB = fabs(tiltFBSmooth) > TILT_FB_THRESHOLD;
  bool warnMpu = warnAccel || warnTiltSide || warnTiltFB;
  bool alert = warnLidar || warnMpu;

  // Relay 16: LED strip 1 ON only when lumens < threshold
  bool fogOn = lumensSmooth < LIGHT_THRESHOLD;
  setFogLight(fogOn);
  if (DEBUG_MODE)
  {
    Serial.print("[Relay] LED Strip 1 (pin 16): ");
    Serial.println(fogOn ? "ON" : "OFF");
  }

  // Relay 17: LED strip 2 + buzzer ON only when alert is active
  setWarningLight(alert);
  setBuzzer(alert);
  if (DEBUG_MODE)
  {
    Serial.print("[Relay] LED Strip 2 + Buzzer (pin 17): ");
    Serial.println(alert ? "ON" : "OFF");
  }

  // Non-blocking upload/print every 1 second
  static unsigned long lastUpload = 0;
  unsigned long now = millis();
  if (now - lastUpload >= 1000)
  {
    lastUpload = now;
    timeClient.update();
    unsigned long epoch = timeClient.getEpochTime();
    struct tm *tm_info = gmtime((time_t *)&epoch);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", tm_info);

    // Firebase upload
    String path = String(FIREBASE_SENSOR_PATH);
    path.concat("/");
    path.concat(timestamp);
    FirebaseJson json;
    json.set("lumens", lumensSmooth);
    json.set("distance", distanceSmooth);
    json.set("accelX", accelXSmooth);
    json.set("tiltSide", tiltSideSmooth);
    json.set("tiltFB", tiltFBSmooth);
    json.set("fogOn", fogOn);
    json.set("warning", alert);
    json.set("buzzer", warnMpu);
    json.set("timestamp", timestamp);
    if (DEBUG_MODE)
    {
      Serial.print("[DEBUG] WiFi status (loop): ");
      Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Not Connected");
      Serial.print("[DEBUG] Firebase ready: ");
      Serial.println(Firebase.ready() ? "Yes" : "No");
    }
    if (Firebase.ready())
    {
      bool fbResult = Firebase.setJSON(fbdo, path.c_str(), json);
      if (DEBUG_MODE)
      {
        Serial.print("[DEBUG] Firebase upload status: ");
        Serial.println(fbResult ? "Success" : "Failed");
        if (!fbResult)
        {
          Serial.print("[DEBUG] Firebase error: ");
          Serial.println(fbdo.errorReason());
        }
      }
    }
    else if (DEBUG_MODE)
    {
      Serial.println("[DEBUG] Firebase not ready (upload skipped)");
    }

    // Debug output
    if (DEBUG_MODE)
    {
      Serial.print("Lumens: ");
      Serial.print(lumensSmooth);
      Serial.print(" | Distance: ");
      Serial.print(distanceSmooth);
      Serial.print(" | AccelX: ");
      Serial.print(accelXSmooth);
      Serial.print(" | TiltSide: ");
      Serial.print(tiltSideSmooth);
      Serial.print(" | TiltFB: ");
      Serial.print(tiltFBSmooth);
      Serial.print(" | Fog: ");
      Serial.print(fogOn);
      Serial.print(" | Warn: ");
      Serial.print(alert);
      Serial.print(" | Buzzer: ");
      Serial.print(warnMpu);
      Serial.print(" | Time: ");
      Serial.println(timestamp);
    }
  }
}
