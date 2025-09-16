#include "actuators.h"
#include "config.h"

void actuatorsInit() {
  pinMode(PIN_RELAY_FOG, OUTPUT);
  pinMode(PIN_RELAY_WARN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  setFogLight(false);
  setWarningLight(false);
  setBuzzer(false);
}

void setFogLight(bool on) {
  digitalWrite(PIN_RELAY_FOG, on ? HIGH : LOW);
}

void setWarningLight(bool on) {
  digitalWrite(PIN_RELAY_WARN, on ? HIGH : LOW);
}

void setBuzzer(bool on) {
  digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
}
