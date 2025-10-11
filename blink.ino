//Mark Antonio - 2025
#define LED_PIN D2   // Use D2 pin

void setup() {
  pinMode(LED_PIN, OUTPUT);   // Set D2 as output
}

void loop() {
  digitalWrite(LED_PIN, HIGH);   // Turn LED ON
  delay(1000);                   // Wait 1 second
  digitalWrite(LED_PIN, LOW);    // Turn LED OFF
  delay(1000);                   // Wait 1 second
}
