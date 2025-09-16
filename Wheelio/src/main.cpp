#include <Arduino.h>
#include "MPU6050_Mock.h"
#include "Lidar_Mock.h"
#include "Config.h"
#include "LightSensor.h"
#include "FogLightRelay.h"
#include "DebugConfig.h"
#include "MPU6050Handler.h"
#include "WarningRelayAndBuzzer.h"

LightSensor lightSensor;
FogLightRelay fogLightRelay;
MPU6050Handler mpuHandler;
WarningRelayAndBuzzer warningRelayAndBuzzer;

// Pin definitions
#define PIN_LIGHT_SENSOR 33
#define PIN_RELAY1 16

// State variables
bool fogLightOn = false;

// Light sensor setup
void setupLightSensor()
{
  pinMode(PIN_LIGHT_SENSOR, INPUT);
}

// Fog light control logic
void taskLightSensor()
{
  int light = analogRead(PIN_LIGHT_SENSOR);
  static unsigned long lastLowLight = 0;

  if (light < LIGHT_SENSOR_THRESHOLD)
  {
    lastLowLight = millis();
  }
  // Only turn on fog light if low light persists
  if (millis() - lastLowLight < LIGHT_SENSOR_PERSIST_TIME)
  {
    digitalWrite(PIN_RELAY1, HIGH);
    fogLightOn = true;
  }
  else
  {
    digitalWrite(PIN_RELAY1, LOW);
    fogLightOn = false;
  }
}

void setup()
{
  #if DEBUG_MODE
  Serial.begin(115200);
  #endif
  lightSensor.setup();
  fogLightRelay.setup();
  mpuHandler.setup();
  warningRelayAndBuzzer.setup();
}

void loop()
{
  lightSensor.task();
  fogLightRelay.control(lightSensor.isFogLightOn());

  mpuHandler.task();
  warningRelayAndBuzzer.control(mpuHandler.isWarningLightOn(), mpuHandler.isBuzzerOn());

  DEBUG_PRINT("Fog Light State: ");
  DEBUG_PRINTLN(lightSensor.isFogLightOn());
  DEBUG_PRINT("Warning Light State: ");
  DEBUG_PRINTLN(mpuHandler.isWarningLightOn());
  DEBUG_PRINT("Buzzer State: ");
  DEBUG_PRINTLN(mpuHandler.isBuzzerOn());

  delay(100);
}