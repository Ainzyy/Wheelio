#include "MPU6050Handler.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include "Config.h"
#include "DebugConfig.h"

Adafruit_MPU6050 mpu; // Updated class name

MPU6050Handler::MPU6050Handler() : speed(0), lastTime(0) {}

void MPU6050Handler::setup() {
    Wire.begin(PIN_MPU6050_SDA, PIN_MPU6050_SCL); // Ensure pins are defined
    mpu.begin();
}

void MPU6050Handler::task() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;

    float gx = g.gyro.x;
    float gy = g.gyro.y;
    float gz = g.gyro.z;

    float ax_g = ax / 16384.0; // Convert to g
    float ay_g = ay / 16384.0;
    float az_g = az / 16384.0;

    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0; // Time in seconds
    lastTime = now;

    speed += ax_g * 9.81 * dt; // Update speed (m/s)
    float speedKmh = speed * 3.6; // Convert to km/h

    // Complementary filter parameters
    const float alpha = COMPLEMENTARY_FILTER_ALPHA; // Use value from Config.h
    // const float dt = 0.01;    // Time step (adjust as needed)

    // Ensure roll is defined
    static float roll = 0.0; // Initialize roll

    // Calculate roll using complementary filter
    float accelRoll = atan2(ay_g, az_g) * 180.0 / 3.14159; // Roll from accelerometer
    roll = alpha * (roll + gx * dt) + (1 - alpha) * accelRoll; // Complementary filter

    // Ensure smoothed variables are defined
    static float smoothedRoll = 0.0;
    static float smoothedSpeed = 0.0;

    // Smoothing parameters
    const float smoothingFactor = SMOOTHING_FACTOR; // Use value from Config.h
    // Apply smoothing to roll
    smoothedRoll = smoothingFactor * roll + (1 - smoothingFactor) * smoothedRoll;
    roll = smoothedRoll;

    // Apply smoothing to speed
    smoothedSpeed = smoothingFactor * speedKmh + (1 - smoothingFactor) * smoothedSpeed;
    speedKmh = smoothedSpeed;

    DEBUG_PRINT("Speed (km/h): ");
    DEBUG_PRINTLN(speedKmh);
    DEBUG_PRINT("Roll (deg): ");
    DEBUG_PRINTLN(roll);

    // Refined warning light and buzzer logic
    // Adjusted for active-low relays
    if (speedKmh > 30.0 || roll < -30.0 || roll > 30.0) {
        warningLightOn = false; // Active low
        buzzerOn = false;       // Active low
    } else {
        warningLightOn = true;  // Active low
        buzzerOn = true;        // Active low
    }
}

bool MPU6050Handler::isWarningLightOn() const {
    return warningLightOn;
}

bool MPU6050Handler::isBuzzerOn() const {
    return buzzerOn;
}