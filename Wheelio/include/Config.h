#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions
constexpr int PIN_LIGHT_SENSOR = 33;
constexpr int PIN_RELAY1 = 16;
constexpr int PIN_RELAY2 = 17;
constexpr int PIN_BUZZER = 5;
constexpr int PIN_MPU6050_SDA = 21;
constexpr int PIN_MPU6050_SCL = 22;

// Light sensor configuration
constexpr int LIGHT_SENSOR_THRESHOLD = 200; // Adjust as needed
constexpr unsigned long LIGHT_SENSOR_PERSIST_TIME = 2000; // ms

// Complementary filter configuration
/**
 * Weight for accelerometer data in the Complementary Filter.
 * Range: 0.0 (all gyroscope) to 1.0 (all accelerometer).
 */
constexpr float COMPLEMENTARY_FILTER_ALPHA = 0.45;

// Smoothing configuration
/**
 * Smoothing factor for roll and speed calculations.
 * Higher values result in faster response.
 */
constexpr float SMOOTHING_FACTOR = 0.3;

// Default states for relays
/**
 * Default state for Relay 1 (Fog Light Relay).
 * Set to LOW (0) to turn off on boot, HIGH (1) to turn on.
 */
constexpr int DEFAULT_RELAY1_STATE = LOW;

/**
 * Default state for Relay 2 (Warning Light Relay).
 * Set to LOW (0) to turn off on boot, HIGH (1) to turn on.
 */
constexpr int DEFAULT_RELAY2_STATE = LOW;

#endif // CONFIG_H