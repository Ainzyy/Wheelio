#ifndef LIGHTSENSOR_H
#define LIGHTSENSOR_H

#include <Arduino.h>

class LightSensor {
public:
    LightSensor(uint8_t pin);
    void begin();
    int readLight();
private:
    uint8_t _pin;
};

#endif // LIGHTSENSOR_H
