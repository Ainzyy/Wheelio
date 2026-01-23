#include "MPU6050Sensor.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "config.h"

static Adafruit_MPU6050 mpu;

MPU6050Sensor::MPU6050Sensor(float filterAlpha) 
    : initialized(false), _alpha(filterAlpha), _tiltFB_zero(0.0f), _tiltSide_zero(0.0f) {
    _prevData = {0,0,0,0,0};
}

bool MPU6050Sensor::begin() {
    // Wire.begin() should be called in main setup
    if (!mpu.begin(ADDR_MPU6050, &Wire)) {
        initialized = false;
        return false;
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    initialized = true;
    return true;
}

float MPU6050Sensor::readTilt() {
    if (!initialized) return 0.0f;
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    float tiltSide = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
    float tiltFB = atan2(a.acceleration.x, a.acceleration.z) * 180.0 / PI;
    // Return one axis for compatibility
    return tiltFB;
}

float MPU6050Sensor::readAcceleration() {
    if (!initialized) return 0.0f;
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    float mag = sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z) / 9.81;
    return mag;
}

MpuData MPU6050Sensor::readMpuData() {
    if (!initialized) return _prevData;
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    float accelX = a.acceleration.x;
    float accelY = a.acceleration.y;
    float accelZ = a.acceleration.z;
    float tiltSide = atan2(accelY, accelZ) * 180.0 / PI;
    float tiltFB = atan2(accelX, accelZ) * 180.0 / PI;
    
    // Apply calibration zero reference
    tiltFB -= _tiltFB_zero;
    tiltSide -= _tiltSide_zero;
    
    _prevData.accelX = _alpha * accelX + (1 - _alpha) * _prevData.accelX;
    _prevData.accelY = _alpha * accelY + (1 - _alpha) * _prevData.accelY;
    _prevData.accelZ = _alpha * accelZ + (1 - _alpha) * _prevData.accelZ;
    _prevData.tiltSide = _alpha * tiltSide + (1 - _alpha) * _prevData.tiltSide;
    _prevData.tiltFB = _alpha * tiltFB + (1 - _alpha) * _prevData.tiltFB;
    return _prevData;
}

void MPU6050Sensor::setTiltZeroReference(float tiltFB, float tiltSide) {
    _tiltFB_zero = tiltFB;
    _tiltSide_zero = tiltSide;
}

void MPU6050Sensor::calibrateLevel() {
    // Capture current tilt angles as zero reference (assumes device is level)
    if (!initialized) return;
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    _tiltFB_zero = atan2(a.acceleration.x, a.acceleration.z) * 180.0 / PI;
    _tiltSide_zero = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
}

void MPU6050Sensor::resetCalibration() {
    _tiltFB_zero = 0.0f;
    _tiltSide_zero = 0.0f;
}
void MPU6050Sensor::setAlpha(float alpha) {
    _alpha = alpha;
}

bool MPU6050Sensor::isInitialized() const {
    return initialized;
}
