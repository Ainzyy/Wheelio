# CJMCU-531 Example Project

This project demonstrates a simple C++ program for the CJMCU-531 distance sensor, measuring distance in centimeters and printing the result to the serial monitor.

## Files
- `src/main.cpp`: Main application code
- `include/CJMCU531.h`: Sensor class header
- `src/CJMCU531.cpp`: Sensor class implementation

## Usage
1. Connect the CJMCU-531 sensor to your ESP32 (I2C: SDA/SCL).
2. Build and upload the project using PlatformIO.
3. Open the serial monitor at 115200 baud to view distance readings.

## Notes
- The current implementation uses a mock value for demonstration. Replace the logic in `CJMCU531::readDistanceCM()` with actual sensor communication for real measurements.
