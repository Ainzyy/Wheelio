#include "light_sensor.h"
#include "config.h"

static float prevLumens = 0;
static const float alpha = 0.2f; // Complementary filter constant

void lightSensorInit() {
  pinMode(PIN_LIGHT_SENSOR, INPUT);
}

float readLightLevel() {
  int analogValue = analogRead(PIN_LIGHT_SENSOR);
  // Example calibration: lumens = a * analog + b
  float lumens = LIGHT_CALIB_A * analogValue + LIGHT_CALIB_B;
  // Complementary filter
  prevLumens = alpha * lumens + (1 - alpha) * prevLumens;
  return prevLumens;
}
