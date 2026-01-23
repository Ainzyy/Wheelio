#include "lidar_sensor.h"
#include <Adafruit_VL53L0X.h>

static Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void lidarInit() {
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1)
      ;
  }
  lox.startRangeContinuous();
}

int readLidarDistance() {
  if (lox.isRangeComplete()) {
    int distance = lox.readRange() / 10; // Convert mm to cm

    // Handle out-of-range values
    if (distance == 0 || distance > 200) { // Assuming 200 cm is the max range
      return -1; // Return -1 for out-of-range
    }

    return distance;
  }
  return -1; // Return -1 if range is not complete
}
