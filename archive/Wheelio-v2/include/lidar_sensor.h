#ifndef LIDAR_SENSOR_H
#define LIDAR_SENSOR_H

#include <Arduino.h>

void lidarInit();
int readLidarDistance(); // returns distance in cm

#endif
