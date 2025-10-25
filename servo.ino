#include <Servo.h>

Servo myservo;  
int servoPin = D4; // GPIO2

void setup() {
  myservo.attach(servoPin,500,2400); // attach servo to pin
  Serial.begin(9600);
}

void loop() {
  // Move to 0 degrees
  myservo.write(0);
  Serial.println("Position: 0°");
  delay(1000);

  // Move to 90 degrees
  myservo.write(90);
  Serial.println("Position: 90°");
  delay(1000);

  // Move to 180 degrees
  myservo.write(180);
  Serial.println("Position: 180°");
  delay(1000);
}
