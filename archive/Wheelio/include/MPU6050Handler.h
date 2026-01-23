#ifndef MPU6050_HANDLER_H
#define MPU6050_HANDLER_H

#include <Wire.h>
#include <Adafruit_MPU6050.h> // Corrected include directive

class MPU6050Handler {
private:
    Adafruit_MPU6050 mpu; // Updated class name
    float speed;
    unsigned long lastTime;
    bool warningLightOn;
    bool buzzerOn;

public:
    MPU6050Handler();
    void setup();
    void task();
    bool isWarningLightOn() const;
    bool isBuzzerOn() const;
};

#endif // MPU6050_HANDLER_H