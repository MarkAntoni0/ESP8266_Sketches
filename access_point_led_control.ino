//Mark Antonio - 2025
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ======= AP CREDENTIALS =======
const char* apSSID = "ESP8266_LED_AP";     // Name of WiFi network
const char* apPASS = "12345678";           // Password (min. 8 chars)

// ======= LED PIN =======
#define LED_PIN D2
bool ledOn = false;

// Web server on port 80
ESP8266WebServer server(80);

// ======= Webpage with two buttons =======
String buildPage() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP8266 LED Control</title></head>";
  html += "<body style='text-align:center; font-family:Arial; margin:30px;'>";
  html += "<h2>ESP8266 LED Control (AP Mode)</h2>";
  html += "<p>LED is <b>";
  html += (ledOn ? "ON" : "OFF");
  html += "</b></p>";
  html += "<a href='/on'><button style='padding:12px 24px; margin:10px;'>Turn ON</button></a>";
  html += "<a href='/off'><button style='padding:12px 24px; margin:10px;'>Turn OFF</button></a>";
  html += "<p style='margin-top:20px; font-size:12px; color:#555;'>Connect to WiFi: <b>";
  html += apSSID;
  html += "</b> (password: ";
  html += apPASS;
  html += ")</p></body></html>";
  return html;
}

// ======= Route Handlers =======
void handleRoot() { server.send(200, "text/html", buildPage()); }
void handleOn()   { ledOn = true;  digitalWrite(LED_PIN, HIGH); server.send(200, "text/html", buildPage()); }
void handleOff()  { ledOn = false; digitalWrite(LED_PIN, LOW);  server.send(200, "text/html", buildPage()); }

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Start WiFi in AP mode
  Serial.println("Starting Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPASS);

  // Show IP
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP started. Connect to WiFi '");
  Serial.print(apSSID);
  Serial.print("' with password '");
  Serial.print(apPASS);
  Serial.print("'. Then open: http://");
  Serial.println(myIP);

  // Routes
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  // Start server
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}
