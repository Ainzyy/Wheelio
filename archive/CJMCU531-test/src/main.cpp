#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>

Adafruit_VL53L1X lox = Adafruit_VL53L1X();

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(1);
  Wire.begin();
  if (!lox.begin(0x29, &Wire)) {
    Serial.println("Failed to find VL53L1X sensor!");
    while (1) delay(10);
  }
  Serial.println("VL53L1X initialized.");
}

void loop() {
  if (!lox.dataReady()) {
    delay(5);
    return;
  }
  int distance = lox.distance();
  if (distance == -1) {
    Serial.println("Read failed!");
  } else {
    Serial.print("Distance: ");
    Serial.print(distance / 10.0, 1); // mm to cm
    Serial.println(" cm");
  }
  lox.clearInterrupt();
  delay(100);
}