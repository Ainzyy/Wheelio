---
# Wheelio Project Documentation
---

## Overview
Wheelio is an ESP32-based smart mobility project that integrates multiple sensors and actuators for enhanced safety and automation. This document serves as a comprehensive reference for hardware, logic, and development notes.

---

## Table of Contents
1. [Overview](#overview)
2. [Hardware Components](#hardware-components)
3. [Pin Configurations](#pin-configurations)
4. [Logic Requirements](#logic-requirements)
5. [Planned Features](#planned-features)
6. [Development Notes](#development-notes)

---

## Hardware Components
- **ESP32 Dev Board**
- **Grove Light Sensor v1.2**
- **MPU6050 Accelerometer & Gyro**
- **Lidar Sensor**
- **Relay Module (2 channels)**
- **Buzzer**

---

## Pin Configurations
| Component                | Pin(s)           | Notes                |
|--------------------------|------------------|----------------------|
| Grove Light Sensor v1.2  | GPIO 33          | Analog Input         |
| MPU6050 (0x68)           | GPIO 21 (SDA)    | I2C Bus              |
|                          | GPIO 22 (SCL)    |                      |
| Lidar (???)              | GPIO 21 (SDA)    | I2C Bus (shared)     |
|                          | GPIO 22 (SCL)    |                      |
| Relay 1 (Fog Light)      | GPIO 16          | Digital Output       |
| Relay 2 (Warning Light)  | GPIO 17          | Digital Output       |
| Buzzer                   | GPIO 5           | Digital Output       |

---

## Architecture & Programming Approach

### Modular Design with Enable Flags and Debug Mode
The Wheelio firmware is structured for modularity and flexibility. Each sensor and actuator is managed by its own setup and task functions. Preprocessor flags allow selective enabling/disabling of components, and a debug mode provides real-time feedback for troubleshooting.

#### Component Enable Flags
Use `#define` flags to control which modules are active:

```c
#define LIGHT_SENSOR_ENABLE   1
#define MPU6050_ENABLE        1
#define LIDAR_ENABLE          1
#define RELAY_ENABLE          1
#define BUZZER_ENABLE         1
#define DEBUG_MODE            1
```

#### Setup Functions
Each component has its own setup function:

```c
void setupLightSensor() { /* Light sensor initialization */ }
void setupMPU6050()    { /* MPU6050 initialization */ }
void setupLidar()      { /* Lidar initialization */ }
void setupRelays()     { /* Relay initialization */ }
void setupBuzzer()     { /* Buzzer initialization */ }
```

#### Task Functions
Each component performs its logic in its own function:

```c
void taskLightSensor() { /* Light sensor logic */ }
void taskMPU6050()     { /* MPU6050 logic */ }
void taskLidar()       { /* Lidar logic */ }
void taskRelays()      { /* Relay logic */ }
void taskBuzzer()      { /* Buzzer logic */ }
```

#### Debug Mode
When `DEBUG_MODE` is enabled, sensor readings and actuator status are printed to the console:

```c
void debugPrint() {
    #if LIGHT_SENSOR_ENABLE
    Serial.println("Light sensor reading: ...");
    #endif
    #if MPU6050_ENABLE
    Serial.println("MPU6050 reading: ...");
    #endif
    #if LIDAR_ENABLE
    Serial.println("Lidar reading: ...");
    #endif
    #if RELAY_ENABLE
    Serial.println("Relay status: ...");
    #endif
    #if BUZZER_ENABLE
    Serial.println("Buzzer status: ...");
    #endif
}
```

#### Main Loop Structure

```c
void setup() {
    #if LIGHT_SENSOR_ENABLE
    setupLightSensor();
    #endif
    #if MPU6050_ENABLE
    setupMPU6050();
    #endif
    #if LIDAR_ENABLE
    setupLidar();
    #endif
    #if RELAY_ENABLE
    setupRelays();
    #endif
    #if BUZZER_ENABLE
    setupBuzzer();
    #endif
}

void loop() {
    #if LIGHT_SENSOR_ENABLE
    taskLightSensor();
    #endif
    #if MPU6050_ENABLE
    taskMPU6050();
    #endif
    #if LIDAR_ENABLE
    taskLidar();
    #endif
    #if RELAY_ENABLE
    taskRelays();
    #endif
    #if BUZZER_ENABLE
    taskBuzzer();
    #endif
    #if DEBUG_MODE
    debugPrint();
    #endif
}
```

**Benefits:**
- Modular design: Easy to maintain and extend
- Selective debugging: Isolate components for testing
- Power efficiency: Disable unused modules
- Scalability: Add new components with minimal changes

---

## Logic Requirements
1. **Fog Light Control (Light Sensor):**
    - If the light sensor detects low ambient light, the fog light should turn on.
    - Handle false negatives: ignore brief or misleading light from other sources at night so the fog light remains on when needed.
2. **Warning Light (Lidar):**
    - If the Lidar sensor detects an object or ground within ~1 meter below, the warning light should turn on.
3. **Warning Light (Acceleration):**
    - If the forward acceleration (from MPU6050) corresponds to a speed greater than 30 km/h, the warning light should turn on.

---

## Example Program: Modular Wheelio Firmware

```c
#include <Wire.h>
#include <MPU6050.h> // Use a suitable MPU6050 library
// #include <LidarLite.h> // Uncomment and use your Lidar library

#define LIGHT_SENSOR_ENABLE   1
#define MPU6050_ENABLE        1
#define LIDAR_ENABLE          1
#define RELAY_ENABLE          1
#define BUZZER_ENABLE         1
#define DEBUG_MODE            1

// Pin definitions
#define PIN_LIGHT_SENSOR 33
#define PIN_MPU6050_SDA 21
#define PIN_MPU6050_SCL 22
#define PIN_LIDAR_SDA   21
#define PIN_LIDAR_SCL   22
#define PIN_RELAY1      16
#define PIN_RELAY2      17
#define PIN_BUZZER      5

#if MPU6050_ENABLE
MPU6050 mpu;
#endif

void setupLightSensor() {
    pinMode(PIN_LIGHT_SENSOR, INPUT);
}

void setupMPU6050() {
#if MPU6050_ENABLE
    Wire.begin(PIN_MPU6050_SDA, PIN_MPU6050_SCL);
    mpu.initialize();
#endif
}

void setupLidar() {
    // Initialize Lidar here (depends on your Lidar library)
}

void setupRelays() {
    pinMode(PIN_RELAY1, OUTPUT);
    pinMode(PIN_RELAY2, OUTPUT);
    digitalWrite(PIN_RELAY1, LOW);
    digitalWrite(PIN_RELAY2, LOW);
}

void setupBuzzer() {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
}

// State variables for logic
bool fogLightOn = false;
bool warningLightOn = false;
bool buzzerOn = false;

void taskLightSensor() {
    int light = analogRead(PIN_LIGHT_SENSOR);
    static unsigned long lastLowLight = 0;
    static const int threshold = 200; // Adjust as needed
    static const unsigned long persistTime = 2000; // ms

    if (light < threshold) {
        lastLowLight = millis();
    }
    // Only turn on fog light if low light persists
    if (millis() - lastLowLight < persistTime) {
        digitalWrite(PIN_RELAY1, HIGH);
        fogLightOn = true;
    } else {
        digitalWrite(PIN_RELAY1, LOW);
        fogLightOn = false;
    }
}

void taskMPU6050() {
#if MPU6050_ENABLE
    mpu.getMotion6(NULL, NULL, NULL, NULL, NULL, NULL);
    float ax = mpu.getAccelerationX() / 16384.0; // g
    float ay = mpu.getAccelerationY() / 16384.0; // g
    float az = mpu.getAccelerationZ() / 16384.0; // g
    float gx = mpu.getRotationX() / 131.0; // deg/s
    float gy = mpu.getRotationY() / 131.0; // deg/s
    float gz = mpu.getRotationZ() / 131.0; // deg/s

    // Calculate speed (simple integration, for demo only)
    static float speed = 0;
    static unsigned long lastTime = 0;
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    lastTime = now;
    speed += ax * 9.81 * dt; // m/s
    float speedKmh = speed * 3.6;

    // Calculate roll (for demo, use ay/az)
    float roll = atan2(ay, az) * 180.0 / 3.14159;

    // Logic for warning light and buzzer
    if (speedKmh > 30.0) {
        digitalWrite(PIN_RELAY2, HIGH);
        warningLightOn = true;
    }
    if (abs(roll) > 30.0) {
        digitalWrite(PIN_RELAY2, HIGH);
        digitalWrite(PIN_BUZZER, HIGH);
        warningLightOn = true;
        buzzerOn = true;
    } else {
        digitalWrite(PIN_BUZZER, LOW);
        buzzerOn = false;
    }
#endif
}

void taskLidar() {
    // Replace with actual Lidar reading
    float distance = getDistanceFromLidar();
    if (distance < 1.0) {
        digitalWrite(PIN_RELAY2, HIGH);
        warningLightOn = true;
    }
}

void taskRelays() {
    // Additional relay logic if needed
    // (e.g., reset warning light if no triggers)
    if (!warningLightOn) {
        digitalWrite(PIN_RELAY2, LOW);
    }
    if (!fogLightOn) {
        digitalWrite(PIN_RELAY1, LOW);
    }
    warningLightOn = false;
    fogLightOn = false;
}

void taskBuzzer() {
    // Additional buzzer logic if needed
    if (!buzzerOn) {
        digitalWrite(PIN_BUZZER, LOW);
    }
    buzzerOn = false;
}

void debugPrint() {
#if LIGHT_SENSOR_ENABLE
    Serial.print("Light: "); Serial.println(analogRead(PIN_LIGHT_SENSOR));
#endif
#if MPU6050_ENABLE
    Serial.print("Speed (km/h): "); Serial.println(getSpeedFromMPU6050());
    Serial.print("Roll (deg): "); Serial.println(getRollFromMPU6050());
#endif
#if LIDAR_ENABLE
    Serial.print("Lidar (m): "); Serial.println(getDistanceFromLidar());
#endif
}

void setup() {
    Serial.begin(115200);
#if LIGHT_SENSOR_ENABLE
    setupLightSensor();
#endif
#if MPU6050_ENABLE
    setupMPU6050();
#endif
#if LIDAR_ENABLE
    setupLidar();
#endif
#if RELAY_ENABLE
    setupRelays();
#endif
#if BUZZER_ENABLE
    setupBuzzer();
#endif
}

void loop() {
#if LIGHT_SENSOR_ENABLE
    taskLightSensor();
#endif
#if MPU6050_ENABLE
    taskMPU6050();
#endif
#if LIDAR_ENABLE
    taskLidar();
#endif
#if RELAY_ENABLE
    taskRelays();
#endif
#if BUZZER_ENABLE
    taskBuzzer();
#endif
#if DEBUG_MODE
    debugPrint();
#endif
    delay(100);
}

// Mock sensor functions for illustration (replace with real sensor code)
float getSpeedFromMPU6050() { return 0.0; }
float getRollFromMPU6050() { return 0.0; }
float getDistanceFromLidar() { return 2.0; }
```



