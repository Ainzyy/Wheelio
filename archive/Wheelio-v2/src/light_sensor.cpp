#include "light_sensor.h"
#include "config.h"

void lightSensorInit() { pinMode(PIN_LIGHT_SENSOR, INPUT); }

float readLightLevel() {
  // Read raw analog value from the light sensor
  return analogRead(PIN_LIGHT_SENSOR);
}
