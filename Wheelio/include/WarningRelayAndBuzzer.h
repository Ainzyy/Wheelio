#ifndef WARNING_RELAY_AND_BUZZER_H
#define WARNING_RELAY_AND_BUZZER_H

class WarningRelayAndBuzzer {
public:
    WarningRelayAndBuzzer();
    void setup();
    void control(bool warningLightState, bool buzzerState);
};

#endif // WARNING_RELAY_AND_BUZZER_H