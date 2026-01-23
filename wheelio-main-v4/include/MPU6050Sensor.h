#ifndef MPU6050SENSOR_H
#define MPU6050SENSOR_H

#include <Arduino.h>

struct MpuData {
    float accelX;
    float accelY;
    float accelZ;
    float tiltSide;
    float tiltFB;
};

class MPU6050Sensor {
public:
    MPU6050Sensor(float filterAlpha = 0.2f);
    bool begin();
    float readTilt();
    float readAcceleration();
    MpuData readMpuData();
    bool isInitialized() const;
    void setAlpha(float alpha);
    void setTiltZeroReference(float tiltFB, float tiltSide);
    void calibrateLevel();
    void resetCalibration();
private:
    bool initialized;
    float _alpha;
    MpuData _prevData;
    float _tiltFB_zero;
    float _tiltSide_zero;
};

#endif // MPU6050SENSOR_H
