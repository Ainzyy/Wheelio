#ifndef MPU6050_SENSOR_H
#define MPU6050_SENSOR_H
#include <Arduino.h>

struct MpuData {
  float accelX, accelY, accelZ;
  float tiltSide, tiltFB;
};

void mpu6050Init();
MpuData readMpuData(); // returns filtered data

#endif
