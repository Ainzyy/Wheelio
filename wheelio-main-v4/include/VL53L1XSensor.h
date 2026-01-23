#ifndef VL53L1XSENSOR_H
#define VL53L1XSENSOR_H

#include <Arduino.h>
#include <Adafruit_VL53L1X.h>

class VL53L1XSensor {
public:
    VL53L1XSensor(float filterAlpha = 0.2f);
    bool begin();
    uint16_t readDistance();
    bool isInitialized() const;
    void setAlpha(float alpha);
    void setCalibrationOffset(int offsetCm);
    int getCalibrationOffset() const;
private:
    Adafruit_VL53L1X _vl53;
    bool initialized;
    float _alpha;
    int _prevDist;
    int _calibrationOffset;
};

#endif // VL53L1XSENSOR_H
