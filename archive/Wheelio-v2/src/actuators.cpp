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
  digitalWrite(PIN_RELAY_FOG, on ? LOW : HIGH); // Active low relay
}

void setWarningLight(bool on) {
  digitalWrite(PIN_RELAY_WARN, on ? LOW : HIGH); // Active low relay
}

void setBuzzer(bool on) {
  digitalWrite(PIN_BUZZER, on ? HIGH : LOW); // Active high buzzer
}
