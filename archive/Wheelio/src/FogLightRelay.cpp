#include "FogLightRelay.h"
#include <Arduino.h>
#include "Config.h"

FogLightRelay::FogLightRelay() {}

void FogLightRelay::setup() {
    pinMode(PIN_RELAY1, OUTPUT);
    digitalWrite(PIN_RELAY1, DEFAULT_RELAY1_STATE); // Use default state from Config.h
}

void FogLightRelay::control(bool state) {
    digitalWrite(PIN_RELAY1, state ? HIGH : LOW);
}