#define TRIG_PIN D5  // GPIO14
#define ECHO_PIN D6  // GPIO12

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  long duration;
  float distance_cm;

  // Clear the trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send a 10µs HIGH pulse to trigger the sensor
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo time (in microseconds)
  duration = pulseIn(ECHO_PIN, HIGH);

  // Convert time to distance
  // Sound speed = 343 m/s => 0.0343 cm/µs
  // Divide by 2 because the wave travels back and forth
  distance_cm = (duration * 0.0343) / 2;

  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

  delay(500);
}
