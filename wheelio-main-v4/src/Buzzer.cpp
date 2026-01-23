#include "Buzzer.h"

Buzzer::Buzzer(uint8_t pin) : _pin(pin), _state(false) {}

void Buzzer::begin() {
    pinMode(_pin, OUTPUT);
    off();
}

void Buzzer::on() {
    digitalWrite(_pin, HIGH);
    _state = true;
}

void Buzzer::off() {
    digitalWrite(_pin, LOW);
    _state = false;
}

bool Buzzer::isOn() const {
    return _state;
}
