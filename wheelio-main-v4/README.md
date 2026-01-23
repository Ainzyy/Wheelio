# Wheelio - Mobility Safety System MVP

An ESP32-based IoT safety system for mobility devices featuring dual-axis tilt detection, proximity sensing, light-based fog light control, and impact detection.

## System Overview

Wheelio monitors four key hazards and activates appropriate safety responses:
- **Collision Detection**: LiDAR-based proximity warning (VL53L1X)
- **Tip-Over Detection**: Dual-axis accelerometer-based tilt sensing (MPU6050)
- **Visibility Enhancement**: Light-triggered fog light (Grove Light Sensor)
- **Impact Detection**: High-G acceleration warning (built into IMU)

## Detection Thresholds

| Hazard Type | Parameter | Threshold | Action |
|------------|-----------|-----------|--------|
| Proximity | Distance | < 200 cm | Activate warning |
| Forward/Back Tilt | FB Angle | > 9° | Activate warning |
| Side Tilt | Side Angle | > 30° | Activate warning |
| Impact | Acceleration | > 2.0 G | Activate warning |
| Darkness | Light Level | < 1000 ADC | Activate fog light |

### Tilt Angle Detection Details

**Dual-Axis Tilt Architecture:**
- **Tilt FB (Forward/Back)**: Calculated from `atan2(accelX, accelZ)`, triggers at **> 9°**
  - Detects forward pitching (nose down) or backward pitching (nose up)
  - Lower threshold suited for tip-over scenarios on inclines
  
- **Tilt Side (Side-to-Side)**: Calculated from `atan2(accelY, accelZ)`, triggers at **> 30°**
  - Detects left/right rolling motion
  - Higher threshold suited for side stability on uneven terrain

**Both axes use absolute value checks** (`abs(tilt) > threshold`), so negative and positive values trigger equally.

## Hardware Configuration

### Sensors
| Component | Model | Protocol | Address | GPIO/Pin |
|-----------|-------|----------|---------|----------|
| LiDAR Distance | VL53L1X | I2C | 0x29 | GPIO 21/22 (I2C) |
| Accelerometer/IMU | MPU6050 | I2C | 0x68 | GPIO 21/22 (I2C) |
| Light Sensor | Grove Light | ADC | - | GPIO 33 |

### Actuators
| Component | Type | Pin | Function |
|-----------|------|-----|----------|
| Fog Light Relay | GPIO Relay | GPIO 16 | Activate when dark |
| Warning Relay | GPIO Relay | GPIO 17 | Activate on hazard |
| Buzzer | GPIO PWM | GPIO 5 | Audio alert on hazard |

### I2C Bus
- **SDA**: GPIO 21
- **SCL**: GPIO 22
- **Shared Bus**: Both VL53L1X and MPU6050 on same I2C bus

## Control Logic

### Fog Light (GPIO 16)
```
IF light_level < 1000 THEN
    fog_relay.ON()
ELSE
    fog_relay.OFF()
```

### Warning System (GPIO 17 + Buzzer GPIO 5)
```
IF (distance < 200cm) OR
   (|tiltFB| > 9°) OR
   (|tiltSide| > 30°) OR
   (acceleration > 2.0G) THEN
    warning_relay.ON()
    buzzer.ON()
ELSE
    warning_relay.OFF()
    buzzer.OFF()
```

## Signal Processing

### Exponential Moving Average (EMA) Filtering
All sensor readings are smoothed using configurable EMA filters:

```
filtered_value = α × current_reading + (1 - α) × previous_reading
```

- **LIDAR Filter Alpha**: 0.2 (faster response for proximity)
- **IMU Filter Alpha**: 0.2 (faster response for impacts)

Where α = 1.0 means **instantaneous** response, and α = 0.0 means **no update**.

## Configuration

All settings are centralized in `include/config.h`:

```cpp
// Hardware Activation Flags
#define ENABLE_LIDAR 1
#define ENABLE_IMU 1
#define ENABLE_LIGHT_SENSOR 1
#define ENABLE_FOG_LIGHT 1
#define ENABLE_WARNING_SYSTEM 1
#define ENABLE_DEBUG_SERIAL 1
#define ENABLE_MOCK_DATA 0  // Set to 1 for simulation mode

// Thresholds
#define LIGHT_THRESHOLD 1000
#define DISTANCE_THRESHOLD_CM 200
#define TILT_FB_THRESHOLD 9.0f
#define TILT_SIDE_THRESHOLD 30.0f
#define ACCEL_THRESHOLD 2.0f

// Distance Calibration
#define DISTANCE_CALIBRATION_OFFSET 0  // cm (offset applied to all readings)

// Tilt Calibration
#define TILT_FB_ZERO_REF 0.0f   // Forward/Back zero reference (degrees)
#define TILT_SIDE_ZERO_REF 0.0f // Side-to-side zero reference (degrees)
```

## Sensor Calibration

### Distance Calibration (VL53L1X)

Corrects systematic distance measurement errors:

```cpp
// Set calibration offset (positive = adds to reading, negative = subtracts)
lidar.setCalibrationOffset(-5);      // Apply -5cm offset
int offset = lidar.getCalibrationOffset(); // Get current offset
```

**Calibration Process:**
1. Place a known reference object at exact 100cm from the sensor
2. Note the reading (e.g., 95cm)
3. Calculate offset: `measured - actual = 95 - 100 = -5cm`
4. Set `DISTANCE_CALIBRATION_OFFSET` to `-5` in config.h
5. Recompile and verify readings are now accurate

### Tilt Calibration (MPU6050)

Establishes zero reference for tilt angles (corrects mounting misalignment):

**Option 1: Auto-calibrate when level (Recommended)**
```cpp
// During setup(), place device level and call:
imu.calibrateLevel();  // Captures current angles as zero reference
```

**Option 2: Manual calibration with specific values**
```cpp
imu.setTiltZeroReference(0.5f, -0.2f);  // FB=0.5°, Side=-0.2°
```

**Option 3: Reset to factory defaults**
```cpp
imu.resetCalibration();  // Resets zero reference to 0°/0°
```

**Example Setup Code:**
```cpp
void setup() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    
#if ENABLE_IMU
    if (imu.begin()) {
        Serial.println("IMU: Calibrating to level position...");
        delay(2000);  // Wait for stable reading
        imu.calibrateLevel();
        Serial.println("IMU: Calibration complete!");
    }
#endif

#if ENABLE_LIDAR
    if (lidar.begin()) {
        // Optional: calibrate to known reference distance
        int refReading = lidar.readDistance();
        int offset = refReading - 100;  // Assuming 100cm reference
        lidar.setCalibrationOffset(offset);
    }
#endif
}
```

## Testing

The project includes comprehensive unit tests (52 tests across 6 categories):

### Test Categories
- **Filters** (3 tests): EMA filter alpha blending
- **Distance** (6 tests): Proximity warning logic
- **Tilt** (12 tests): FB/Side threshold detection with boundary conditions
- **Light** (9 tests): Fog light activation logic
- **Actuators** (14 tests): Warning trigger scenarios
- **Mock Data** (8 tests): Sensor simulation patterns

### Running Tests
```bash
cd /path/to/wheelio-main-v4
pio test
```

### Mock Data Mode
Enable simulation without hardware:
```cpp
#define ENABLE_MOCK_DATA 1  // in config.h
```

Mock data patterns:
- **Light**: 1100 ± 400 (sin oscillation)
- **Distance**: 300 ± 200 (cos oscillation)
- **Tilt**: ±40° (sin oscillation)
- **Acceleration**: 1.0 ± 1.5 G (abs sin oscillation)

## Project Structure

```
wheelio-main-v4/
├── include/
│   ├── config.h              # Centralized configuration
│   ├── VL53L1XSensor.h       # LiDAR driver
│   ├── MPU6050Sensor.h       # IMU driver
│   ├── LightSensor.h         # Light sensor driver
│   ├── Relay.h               # GPIO relay control
│   └── Buzzer.h              # Buzzer control
├── src/
│   ├── main.cpp              # Application entry point
│   ├── VL53L1XSensor.cpp     # LiDAR implementation
│   ├── MPU6050Sensor.cpp     # IMU implementation
│   ├── LightSensor.cpp       # Light sensor implementation
│   ├── Relay.cpp             # Relay implementation
│   └── Buzzer.cpp            # Buzzer implementation
├── test/
│   ├── test_all.cpp          # Comprehensive test suite
│   └── test_helpers.h        # Shared test utilities
├── platformio.ini            # Build configuration
└── README.md                 # This file
```

## Architecture Highlights

### Modular Design
- Each hardware component isolated into dedicated class
- Enable/disable features via config flags without recompilation
- Mock data support for testing without hardware

### Single I2C Bus
- Both I2C sensors (VL53L1X and MPU6050) on shared bus (GPIO 21/22)
- Single `Wire.begin()` call in main.cpp
- No address conflicts (0x29 vs 0x68)

### Dual-Actuator Control
- **Fog Light**: Independent on/off logic based on light level
- **Warning System**: Triggers on any hazard condition
- Both can operate simultaneously

### Non-Blocking Architecture
- Sensor polling at 100ms intervals (configurable)
- No blocking delays in main loop
- Suitable for real-time systems

## Quick Start

### 1. Build and Upload
```bash
cd /path/to/wheelio-main-v4
pio run --target upload
```

### 2. Monitor Serial Output
```bash
pio device monitor --baud 115200
```

Expected output (Real mode):
```
[R] L:1050 | D:450 | TFB:2.3 | TS:5.1 | A:1.05
[R] L:980 | D:320 | TFB:8.9 | TS:28.5 | A:1.12
[R] L:750 | D:150 | TFB:12.5 | TS:35.2 | A:2.5  <- Warning triggered
```

Expected output (Mock mode):
```
[M] L:1150 | D:450 | TFB:15.0 | TS:20.3 | A:1.25
[M] L:800 | D:250 | TFB:35.7 | TS:8.2 | A:2.1   <- Warning triggered
```

### 3. Run Tests
```bash
pio test
```

## Development

### Modifying Thresholds
Edit `include/config.h` and rebuild:
```cpp
#define TILT_FB_THRESHOLD 12.0f    // Increase forward tilt threshold
#define TILT_SIDE_THRESHOLD 25.0f  // Decrease side tilt threshold
```

### Adding New Sensors
1. Create sensor class in `include/` and `src/`
2. Add enable flag in `config.h`
3. Add instantiation and reading logic in `src/main.cpp`
4. Add tests in `test/test_all.cpp`

### Disabling Components for Debugging
```cpp
// config.h
#define ENABLE_LIDAR 0          // Skip LiDAR initialization
#define ENABLE_WARNING_SYSTEM 0 // Test only fog light
```

## Libraries

| Library | Version | Purpose |
|---------|---------|---------|
| Adafruit_VL53L1X | 3.1.2 | Time-of-flight distance sensor |
| Adafruit_MPU6050 | 2.2.6 | 6-axis accelerometer/gyro |
| Arduino | Latest | Core ESP32 support |
| Unity | Built-in | Unit testing framework |

## Notes

- **Temperature Compensation**: Not currently implemented; affects tilt angle calculation
- **Calibration**: IMU assumes level ground at startup
- **Response Time**: ~100ms sensor polling + filter delay (typical ~50ms with α=0.2)
- **Power**: Current draw ~100mA (depends on active components)

## License

Wheelio Capstone Project - 2024
