//Mark Antonio - 2025
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ======= YOUR WIFI CREDENTIALS =======
const char* ssid     = "WE_FA6126";
const char* password = "7091b5b2";

// ======= LED PIN =======
#define LED_PIN D2
bool ledOn = false;

// Web server on port 80
ESP8266WebServer server(80);

// ======= Build the HTML page =======
String buildPage() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP8266 LED Control</title></head>";
  html += "<body style='text-align:center; font-family:Arial; margin:30px;'>";
  html += "<h2>ESP8266 LED Control (Station Mode)</h2>";

  html += "<p>LED is <b>";
  html += (ledOn ? "ON" : "OFF");
  html += "</b></p>";

  html += "<a href='/on'><button style='padding:12px 24px; margin:10px;'>Turn ON</button></a>";
  html += "<a href='/off'><button style='padding:12px 24px; margin:10px;'>Turn OFF</button></a>";

  html += "<p style='margin-top:20px; font-size:12px; color:#555;'>Connected to WiFi: <b>";
  html += ssid;
  html += "</b></p></body></html>";

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

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("ESP8266 IP Address: http://");
  Serial.println(WiFi.localIP());

  // Define routes
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  // Start web server
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();
}
