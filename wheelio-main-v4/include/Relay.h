#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

class Relay {
public:
    // polarity: true = ACTIVE_HIGH (ON when pin=HIGH), false = ACTIVE_LOW (ON when pin=LOW)
    Relay(uint8_t pin, bool activeHigh = true);
    void begin();
    void on();
    void off();
    bool isOn() const;
    void setPolarity(bool activeHigh);
    bool getPolarity() const;
private:
    uint8_t _pin;
    bool _state;
    bool _activeHigh;  // true = ACTIVE_HIGH, false = ACTIVE_LOW
};

#endif // RELAY_H
