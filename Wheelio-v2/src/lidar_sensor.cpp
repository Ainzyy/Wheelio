#include "lidar_sensor.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_VL53L1X.h>

static Adafruit_VL53L1X vl53 = Adafruit_VL53L1X();
static int prevDist = 0;
static const float alpha = 0.2f;

void lidarInit() {
  Wire.begin(PIN_LIDAR_SDA, PIN_LIDAR_SCL);
  vl53.begin(0x29, &Wire);
  vl53.startRanging();
}

int readLidarDistance() {
  if (vl53.dataReady()) {
    int dist = vl53.distance() / 10; // mm to cm
    prevDist = alpha * dist + (1 - alpha) * prevDist;
    vl53.clearInterrupt();
  }
  return prevDist;
}
