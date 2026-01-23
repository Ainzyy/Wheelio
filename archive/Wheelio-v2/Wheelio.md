## Hardware Components
- **ESP32 Dev Board**
- **Grove Light Sensor v1.2**
- **MPU6050 Accelerometer & Gyroscope**
- **VL53L1X Lidar Sensor**
- **2-Channel Relay Module**
- **Buzzer**

---

## Pin Configurations

| Component                | Pin(s)           | Description          |
|--------------------------|------------------|----------------------|
| Grove Light Sensor v1.2  | GPIO 33          | Analog input         |
| MPU6050 (I2C address: 0x68) | GPIO 21 (SDA)  | I2C bus              |
|                          | GPIO 22 (SCL)    |                      |
| VL53L1X Lidar (I2C address: 0x29) | GPIO 21 (SDA) | Shared I2C bus      |
|                          | GPIO 22 (SCL)    |                      |
| Relay 1 (Fog Light)      | GPIO 16          | Digital output       |
| Relay 2 (Warning Light)  | GPIO 17          | Digital output       |
| Buzzer                   | GPIO 5           | Digital output       |

---

## Logic Requirements

### 1. Fog Light Control (Light Sensor)
- Activate fog light when ambient light is below threshold.
- Implement filtering to avoid false negatives from transient light sources.

### 2. Warning Light Trigger (Lidar)
- Activate warning light when Lidar detects ground or object within ~2 meters.

### 3. Warning Light & Buzzer Trigger (MPU6050)
- **Acceleration**: Trigger warning light and buzzer if forward acceleration exceeds 2 m/s².
- **Tilt Detection**:
  - Side tilt > 30° (left or right): trigger warning light and buzzer.
  - Forward/backward tilt > 9°: trigger warning light and buzzer.

---

## Libraries

- `Adafruit_MPU6050`
- `Adafruit_VL53L1X`

---

## Thresholds

| Parameter       | Threshold         |
|----------------|-------------------|
| Front/Back Tilt | > 9°              |
| Side Tilt       | > 30° (each side) |
| Acceleration    | > 2 m/s²          |
| Distance        | < 200 cm          |
| Light Level     | < 1000 lumens     |

> Note: Convert Grove analog values to lumens using calibration.


---

## Implementation Notes

1. **Sensor Filtering**  
   Apply complementary filters to all sensor inputs to enhance signal stability and suppress transient noise.

2. **Modular Architecture**  
   Organize the codebase so each hardware component is implemented in its own source file, promoting maintainability and scalability.

3. **Centralized Configuration**  
   Maintain a dedicated configuration file to define:
   - Pin assignments  
   - Threshold values  
   - Sensor calibration parameters  

4. **Debug Mode**  
   Integrate a toggleable debug mode to output real-time sensor data and system states for diagnostics and testing.

> Items 1–4 are already implemented.

5. **Wi-Fi Configuration**  
   Use the `WiFiManager` library to handle network setup. Configure the ESP32 boot button to trigger the Wi-Fi configuration portal.

6. **Cloud Logging**  
   Upload timestamped sensor readings to Firebase. Use NTP synchronization (GMT+8) to ensure accurate time-based logging.

---