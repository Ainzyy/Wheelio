#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <math.h>

// Thresholds from config.h
#define DISTANCE_THRESHOLD_CM 200
#define TILT_FB_THRESHOLD 9.0f
#define TILT_SIDE_THRESHOLD 30.0f
#define ACCEL_THRESHOLD 2.0f
#define LIGHT_THRESHOLD 1000

// --- HELPER FUNCTIONS ---

bool triggerCollisionWarning(uint16_t distance) {
    return (distance < DISTANCE_THRESHOLD_CM);
}

bool triggerTiltWarning(float tiltFB, float tiltSide) {
    // Forward/back tilt > 9° OR side tilt > 30°
    bool fbWarn = (abs(tiltFB) > TILT_FB_THRESHOLD);
    bool sideWarn = (abs(tiltSide) > TILT_SIDE_THRESHOLD);
    return fbWarn || sideWarn;
}

bool triggerFogLight(int light) {
    return (light < LIGHT_THRESHOLD);
}

bool shouldActivateFogLight(int light) {
    return (light < LIGHT_THRESHOLD);
}

bool shouldActivateWarning(uint16_t distance, float tiltFB, float tiltSide, float accelMag) {
    bool proximityWarn = (distance < DISTANCE_THRESHOLD_CM);
    bool tiltFBWarn = (abs(tiltFB) > TILT_FB_THRESHOLD);
    bool tiltSideWarn = (abs(tiltSide) > TILT_SIDE_THRESHOLD);
    bool impactWarn = (abs(accelMag - 1.0f) > (ACCEL_THRESHOLD - 1.0f));
    return (proximityWarn || tiltFBWarn || tiltSideWarn || impactWarn);
}

float applyFilter(float current, float previous, float alpha) {
    return (alpha * current) + (1.0f - alpha) * previous;
}

struct MockSensorData {
    int light;
    uint16_t distance;
    float tilt;
    float accelMag;
};

MockSensorData generateMockData(unsigned long currentTime) {
    MockSensorData data;
    data.light = 1100 + 400 * sin(currentTime / 5000.0);
    data.distance = 300 + 200 * cos(currentTime / 3000.0);
    data.tilt = 40.0 * sin(currentTime / 2000.0);
    data.accelMag = 1.0 + 1.5 * abs(sin(currentTime / 4000.0));
    return data;
}

#endif // TEST_HELPERS_H
