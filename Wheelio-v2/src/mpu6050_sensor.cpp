#include "mpu6050_sensor.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

static Adafruit_MPU6050 mpu;
static MpuData prevData = {0,0,0,0,0};
static const float alpha = 0.2f;

void mpu6050Init() {
  Wire.begin(PIN_MPU_SDA, PIN_MPU_SCL);
  mpu.begin();
}

MpuData readMpuData() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float accelX = a.acceleration.x;
  float accelY = a.acceleration.y;
  float accelZ = a.acceleration.z;
  // Tilt calculations
  float tiltSide = atan2(accelY, accelZ) * 180.0 / PI;
  float tiltFB = atan2(accelX, accelZ) * 180.0 / PI;
  // Complementary filter
  prevData.accelX = alpha * accelX + (1 - alpha) * prevData.accelX;
  prevData.accelY = alpha * accelY + (1 - alpha) * prevData.accelY;
  prevData.accelZ = alpha * accelZ + (1 - alpha) * prevData.accelZ;
  prevData.tiltSide = alpha * tiltSide + (1 - alpha) * prevData.tiltSide;
  prevData.tiltFB = alpha * tiltFB + (1 - alpha) * prevData.tiltFB;
  return prevData;
}
