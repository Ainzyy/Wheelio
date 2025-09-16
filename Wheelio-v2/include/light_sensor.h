#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H
#include <Arduino.h>

void lightSensorInit();
float readLightLevel(); // returns lumens (filtered)

#endif
