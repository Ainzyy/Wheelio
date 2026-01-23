#ifndef CONFIG_H
#define CONFIG_H

// --- Hardware Activation Flags ---
// Set to 1 to enable, 0 to disable for targeted debugging
#define ENABLE_LIDAR 1
#define ENABLE_IMU 1
#define ENABLE_LIGHT_SENSOR 1
#define ENABLE_FOG_LIGHT 1    // Fog relay
#define ENABLE_WARNING_SYSTEM 1 // Warning relay and buzzer
#define ENABLE_DEBUG_SERIAL 1
#define ENABLE_MOCK_DATA 0 // Set to 1 to simulate sensor data

// --- Pin mappings ---
// I2C Shared Pins
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

// Keep specific names for compatibility but map to shared definitions
#define PIN_VL53L1X_SDA PIN_I2C_SDA
#define PIN_VL53L1X_SCL PIN_I2C_SCL
#define PIN_MPU6050_SDA PIN_I2C_SDA
#define PIN_MPU6050_SCL PIN_I2C_SCL
#define PIN_LIGHT_SENSOR 33
#define PIN_FOG_RELAY 16
#define PIN_WARNING_RELAY 17
#define PIN_BUZZER 5

// --- Logic Thresholds ---
#define LIGHT_THRESHOLD           1000  // Ambient light level for fog lights
#define DISTANCE_THRESHOLD_CM     200   // Obstacle distance for warning (cm)
#define TILT_FB_THRESHOLD         9.0f  // Forward/Back tilt threshold (degrees)
#define TILT_SIDE_THRESHOLD       30.0f // Side tilt threshold (degrees)
#define ACCEL_THRESHOLD           2.0f  // G-force for impact/acceleration warning

// --- Sensor Calibration & Filtering ---
#define SENSOR_POLL_INTERVAL_MS 100
#define FILTER_ALPHA_LIDAR 0.2f
#define FILTER_ALPHA_IMU   0.2f

// --- Distance Calibration ---
// Offset applied to all distance readings (positive = adds to reading, negative = subtracts)
#define DISTANCE_CALIBRATION_OFFSET 0  // cm (set to measured offset between sensor and reference)

// --- Tilt Calibration ---
// Zero reference angles when system is level/calibrated (degrees)
#define TILT_FB_ZERO_REF 0.0f   // Forward/Back zero reference
#define TILT_SIDE_ZERO_REF 0.0f // Side-to-side zero reference

// --- Relay Polarity Configuration ---
// Set to 1 for ACTIVE_HIGH (relay ON when pin=HIGH), 0 for ACTIVE_LOW (relay ON when pin=LOW)
#define FOG_LIGHT_ACTIVE_HIGH 0      // Fog light relay polarity
#define WARNING_SYSTEM_ACTIVE_HIGH 0 // Warning relay polarity

// --- WiFi Configuration ---
#define ENABLE_WIFI 1                     // Enable WiFi connectivity
#define WIFI_PORTAL_TIMEOUT_SEC 60        // Time to wait for WiFi connection attempt before opening portal (seconds)
#define WIFI_RECONNECT_INTERVAL_MS 5000   // Interval to check WiFi connection (milliseconds)
#define PIN_BOOT_BUTTON 0                 // ESP32 BOOT button (GPIO 0) for triggering WiFi portal
#define WIFI_DEVICE_NAME "Wheelio-ESP32"  // Device name for WiFi portal

// --- Firebase Configuration ---
#define ENABLE_FIREBASE 1                 // Enable Firebase RTDB connectivity
#define FIREBASE_API_KEY "AIzaSyDg9Le8N-x1v30_ArHJWQGoEIoNdUXStjI"
#define FIREBASE_EMAIL "wheelioooh@gmail.com"
#define FIREBASE_PASSWORD "wheelioGroup?!"
#define FIREBASE_HOST "https://wheelio-0o-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_DB_SECRET "TXcU1wjyEgDD6VAX9QplE5pdVPo8bmjZcj177OdG"
#define FIREBASE_SENSOR_PATH "/sensor_readings_v4"
#define FIREBASE_COMMAND_PATH "/commands"
#define FIREBASE_STATUS_PATH "/device_status"
#define FIREBASE_UPLOAD_INTERVAL_SEC 10   // Send telemetry data every 10 seconds
#define DEVICE_ID "wheelio_001"           // Unique device identifier

// --- NTP Configuration (for Firebase timestamps) ---
#define ENABLE_NTP 1                      // Enable NTP for accurate time
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 28800              // GMT+8 (Asia/Singapore)
#define DAYLIGHT_OFFSET_SEC 0

// --- Sensor Addresses ---
#define ADDR_VL53L1X 0x29
#define ADDR_MPU6050 0x68

#endif // CONFIG_H
