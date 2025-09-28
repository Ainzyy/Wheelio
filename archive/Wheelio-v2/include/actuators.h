#ifndef ACTUATORS_H
#define ACTUATORS_H
#include <Arduino.h>

void actuatorsInit();
void setFogLight(bool on);
void setWarningLight(bool on);
void setBuzzer(bool on);

#endif
