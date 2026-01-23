#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
public:
    Buzzer(uint8_t pin);
    void begin();
    void on();
    void off();
    bool isOn() const;
private:
    uint8_t _pin;
    bool _state;
};

#endif // BUZZER_H
