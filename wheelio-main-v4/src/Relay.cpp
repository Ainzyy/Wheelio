#include "Relay.h"

Relay::Relay(uint8_t pin, bool activeHigh) : _pin(pin), _state(false), _activeHigh(activeHigh) {}

void Relay::begin() {
    pinMode(_pin, OUTPUT);
    off();
}

void Relay::on() {
    // Apply polarity: ACTIVE_HIGH writes HIGH, ACTIVE_LOW writes LOW
    digitalWrite(_pin, _activeHigh ? HIGH : LOW);
    _state = true;
}

void Relay::off() {
    // Apply polarity: ACTIVE_HIGH writes LOW, ACTIVE_LOW writes HIGH
    digitalWrite(_pin, _activeHigh ? LOW : HIGH);
    _state = false;
}

bool Relay::isOn() const {
    return _state;
}

void Relay::setPolarity(bool activeHigh) {
    _activeHigh = activeHigh;
}

bool Relay::getPolarity() const {
    return _activeHigh;
}
