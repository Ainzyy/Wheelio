#include "LightSensor.h"
#include <Arduino.h>
#include "Config.h"

LightSensor::LightSensor() : lastLowLight(0), fogLightOn(false) {}

void LightSensor::setup() {
    pinMode(PIN_LIGHT_SENSOR, INPUT);
}

void LightSensor::task() {
    int light = analogRead(PIN_LIGHT_SENSOR);

    // Adjust logic for active low sensor
    if (light > LIGHT_SENSOR_THRESHOLD) {
        lastLowLight = millis();
    }

    if (millis() - lastLowLight < LIGHT_SENSOR_PERSIST_TIME) {
        fogLightOn = true;
    } else {
        fogLightOn = false;
    }
}

bool LightSensor::isFogLightOn() const {
    return fogLightOn;
}