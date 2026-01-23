# Wheelio IoT Project

## Overview
Wheelio is an ESP32-based IoT system designed for real-time safety and automation in mobility devices. It integrates multiple sensors and actuators to monitor environmental and motion conditions, providing automated lighting and warning feedback.

---

## Hardware Components

### Sensors
| Sensor | Purpose | Library |
| --- | --- | --- |
| VL53L1X (LiDar) | Distance/obstacle detection | Adafruit VL53L1X |
| MPU6050 (Accel/Gyro) | Tilt and acceleration | Adafruit MPU6050 |
| Light Sensor (Grove v1.2) | Ambient light | Analog Read |

### Actuators
- **Active Buzzer**: Audible warning
- **5V DC Relay (Active High) x2**:
    - Fog light switch
    - Warning light switch

### Pin Mapping
| Component | ESP32 Pin |
| --- | --- |
| VL53L1X SDA | 21 |
| VL53L1X SCL | 22 |
| MPU6050 SDA | 21 |
| MPU6050 SCL | 22 |
| Light Sensor | 33 |
| Fog Light Relay | 16 |
| Warning Light Relay | 17 |
| Buzzer | 5 |

---

## System Architecture

```
                 +-------------------+
                 |    Wheelio ESP32  |
                 +-------------------+
                 |                   |
                 |  [Sensors]        |
                 |   - VL53L1X       |
                 |   - MPU6050       |
                 |   - Light Sensor  |
                 |                   |
                 |  [Actuators]      |
                 |   - Fog Relay     |
                 |   - Warn Relay    |
                 |   - Buzzer        |
                 +-------------------+
                 |  [Config Module]  |
                 +-------------------+
                 |  [Main Logic]     |
                 +-------------------+
                 |  [Unit Tests]     |
                 +-------------------+
```

---

## Software Design

### 1. Centralized Configuration
- All pin mappings, thresholds, and calibration constants are defined in `include/config.h`.

### 2. Modular Hardware Abstraction
- Each hardware component is encapsulated in a C++ class:
    - `VL53L1XSensor` (LiDar)
    - `MPU6050Sensor` (Accel/Gyro)
    - `LightSensor`
    - `Relay` (for both fog and warning relays)
    - `Buzzer`
- Headers in `include/`, implementations in `src/`.

### 3. Main Application Logic
- `main.cpp` integrates all modules:
    - Initializes sensors and actuators
    - Reads sensor data
    - Applies complementary filtering
    - Controls actuators based on logic
    - Outputs debug info for development

### 4. Signal Processing & Logic
- Complementary filtering is applied to sensor readings to reduce noise.
- Threshold-based logic triggers actuators:
    - **Fog Light**: On if light < 1000 lumens
    - **Warning Light**: On if distance < 200 cm or tilt/acceleration exceeds thresholds
    - **Buzzer**: On if tilt or acceleration exceeds thresholds

### 5. Unit Testing & Mocking
- Unit tests are placed in `test/`.
- Mock sensor data is used to validate logic without hardware.

### 6. Documentation & Code Quality
- All class interfaces are documented in headers.
- Code is modular, organized, and easy to maintain.

---

## Development Workflow
1. Define all constants and pins in `config.h`.
2. Implement and test each hardware class individually.
3. Integrate modules in `main.cpp` and verify system behavior.
4. Write and run unit tests with both real and mock data.
5. Use serial debug output for troubleshooting.

---

## MVP Feature Checklist
- [ ] Modular code for each hardware component
- [ ] Centralized configuration (`config.h`)
- [ ] Real-time sensor reading, filtering, and actuator control
- [ ] Debug output for development/testing
- [ ] Unit tests and mock data in `test/`

---

## Future Features (Post-MVP)

- Wi-Fi configuration and captive portal (WiFiManager)
- Cloud logging (Firebase, NTP)
- Security: Move credentials out of source code
- Robust error handling for hardware and connectivity
- Configurability: Update thresholds/calibration remotely
- OTA firmware updates
- User feedback (status LEDs, display, app integration)
- Expanded documentation (setup, troubleshooting, usage)
