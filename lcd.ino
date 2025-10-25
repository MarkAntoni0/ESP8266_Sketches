//Make sure to download the library LiquidCrystal_I2C by Frank de Brabander
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD I2C address (usually 0x27 or 0x3F) and size (16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Wire.begin(D2, D1); // SDA = D2, SCL = D1 (for ESP8266)
  
  lcd.init();        // Initialize LCD
  lcd.backlight();   // Turn on backlight
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("Hello ESP8266!");

  lcd.setCursor(0, 1);
  lcd.print("LCD via I2C");
}
