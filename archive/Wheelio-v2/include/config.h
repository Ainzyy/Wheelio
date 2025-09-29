#ifndef CONFIG_H
#define CONFIG_H

// --- Pin Assignments ---
#define PIN_LIGHT_SENSOR 33
#define PIN_MPU_SDA 21
#define PIN_MPU_SCL 22
#define PIN_LIDAR_SDA 21
#define PIN_LIDAR_SCL 22
#define PIN_RELAY_FOG 16
#define PIN_RELAY_WARN 17
#define PIN_BUZZER 5

// --- Thresholds ---
extern float TILT_SIDE_THRESHOLD;
extern float TILT_FB_THRESHOLD;
extern float DIST_THRESHOLD;
extern float LIGHT_THRESHOLD;

// --- Debug Mode ---
#define DEBUG_MODE true

// --- WiFiManager ---
#define WIFI_AP_NAME "Wheelio-Setup"
#define WIFI_AP_PASSWORD ""

// --- Firebase ---
#define API_KEY "AIzaSyDg9Le8N-x1v30_ArHJWQGoEIoNdUXStjI"
#define EMAIL "wheelioooh@gmail.com"
#define PASSWORD "wheelioGroup?!"
#define FIREBASE_HOST                                                          \
  "https://wheelio-0o-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH ""
#define FIREBASE_SENSOR_PATH "/sensor_readings"

// --- EMA Filter Sensitivity ---
#define EMA_ALPHA_LIGHT 0.2f // Sensitivity for light sensor
#define EMA_ALPHA_LIDAR 0.15f // Sensitivity for lidar sensor
#define EMA_ALPHA_MPU 0.35f   // Sensitivity for MPU6050 sensor

#endif // CONFIG_H