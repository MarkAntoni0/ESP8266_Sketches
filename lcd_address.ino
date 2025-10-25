#include <Wire.h>

void setup() {
  Wire.begin(D2, D1);
  Serial.begin(9600);
  Serial.println("Scanning...");
  for (byte i = 8; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C Address: 0x");
      Serial.println(i, HEX);
    }
  }
}

void loop() {}
