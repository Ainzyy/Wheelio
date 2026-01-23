#include "VL53L1XSensor.h"
#include <Wire.h>
#include "config.h"

VL53L1XSensor::VL53L1XSensor(float filterAlpha) 
    : _vl53(), initialized(false), _alpha(filterAlpha), _prevDist(0), _calibrationOffset(0) {}

bool VL53L1XSensor::begin() {
    // Wire.begin() is handled in main.cpp for shared I2C bus
    if (!_vl53.begin(ADDR_VL53L1X, &Wire)) {
        initialized = false;
        return false;
    }
    // Configuration removed to match working sample and fix compilation error
    _vl53.startRanging();
    initialized = true;
    return true;
}

uint16_t VL53L1XSensor::readDistance() {
    if (!initialized) return 0;
    if (_vl53.dataReady()) {
        int dist = _vl53.distance() / 10; // mm to cm
        // Apply calibration offset
        dist += _calibrationOffset;
        // Check for valid range before filtering (optional, but good practice)
        if (dist != -1) { 
             _prevDist = _alpha * dist + (1 - _alpha) * _prevDist;
        }
        _vl53.clearInterrupt();
    }
    return (uint16_t)_prevDist;
}

void VL53L1XSensor::setAlpha(float alpha) {
    _alpha = alpha;
}

void VL53L1XSensor::setCalibrationOffset(int offsetCm) {
    _calibrationOffset = offsetCm;
}

int VL53L1XSensor::getCalibrationOffset() const {
    return _calibrationOffset;
}

bool VL53L1XSensor::isInitialized() const {
    return initialized;
}
