#ifndef CONFIG_H
#define CONFIG_H

#include <cstdlib> // For getenv
#include <fstream>
#include <map>
#include <string>

// Declare the env variable
extern std::map<std::string, std::string> env;

// Declare the parseEnvFile function
std::map<std::string, std::string> parseEnvFile(const std::string &filePath);

// Function to load the .env file
void loadEnv();

// Pin assignments
#define PIN_LIGHT_SENSOR 33
#define PIN_MPU_SDA 21
#define PIN_MPU_SCL 22
#define PIN_LIDAR_SDA 21
#define PIN_LIDAR_SCL 22
#define PIN_RELAY_FOG 16
#define PIN_RELAY_WARN 17
#define PIN_BUZZER 5

// Thresholds
#define TILT_SIDE_THRESHOLD 30.0f // degrees
#define TILT_FB_THRESHOLD 9.0f    // degrees
#define ACCEL_THRESHOLD 2.0f      // m/s^2
#define DIST_THRESHOLD 120        // cm
#define LIGHT_THRESHOLD 1000      // lumens (calibrated)

// Calibration (example values, adjust as needed)
#define LIGHT_CALIB_A 1.0f
#define LIGHT_CALIB_B 0.0f

// Debug mode
#define DEBUG_MODE true

// WiFiManager
#define WIFI_AP_NAME "Wheelio-Setup"
#define WIFI_AP_PASSWORD ""

// Firebase
#define API_KEY "AIzaSyDg9Le8N-x1v30_ArHJWQGoEIoNdUXStjI"
// Firebase Account
#define EMAIL "wheelioooh@gmail.com"
#define PASSWORD "wheelioGroup?!"
#define FIREBASE_HOST "https://wheelio-0o-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH ""
#define FIREBASE_SENSOR_PATH "/sensor_readings"

// NTP
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 28800 // GMT+8
#define DAYLIGHT_OFFSET_SEC 0

#endif // CONFIG_H