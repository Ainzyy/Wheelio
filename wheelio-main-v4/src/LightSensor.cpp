#include "LightSensor.h"

LightSensor::LightSensor(uint8_t pin) : _pin(pin) {}

void LightSensor::begin() {
    // No initialization needed for analog read
}

int LightSensor::readLight() {
    return analogRead(_pin);
}
