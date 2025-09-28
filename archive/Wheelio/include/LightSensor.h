#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>

class LightSensor {
private:
    unsigned long lastLowLight;
    bool fogLightOn;

public:
    LightSensor();
    void setup();
    void task();
    bool isFogLightOn() const;
};

#endif // LIGHT_SENSOR_H