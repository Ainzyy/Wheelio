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
  digitalWrite(PIN_RELAY_FOG, on ? LOW : HIGH);
}

void setWarningLight(bool on) {
  digitalWrite(PIN_RELAY_WARN, on ? LOW : HIGH);
}

void setBuzzer(bool on) {
  digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
}
