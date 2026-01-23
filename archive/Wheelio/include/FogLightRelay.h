#ifndef FOG_LIGHT_RELAY_H
#define FOG_LIGHT_RELAY_H

class FogLightRelay {
public:
    FogLightRelay();
    void setup();
    void control(bool state);
};

#endif // FOG_LIGHT_RELAY_H