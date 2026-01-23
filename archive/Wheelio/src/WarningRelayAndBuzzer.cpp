#include "WarningRelayAndBuzzer.h"
#include <Arduino.h>
#include "Config.h"

WarningRelayAndBuzzer::WarningRelayAndBuzzer() {}

void WarningRelayAndBuzzer::setup() {
    pinMode(PIN_RELAY2, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_RELAY2, DEFAULT_RELAY2_STATE); // Use default state from Config.h
    digitalWrite(PIN_BUZZER, LOW);
}

void WarningRelayAndBuzzer::control(bool warningLightState, bool buzzerState) {
    digitalWrite(PIN_RELAY2, warningLightState ? HIGH : LOW);
    digitalWrite(PIN_BUZZER, buzzerState ? HIGH : LOW);
}