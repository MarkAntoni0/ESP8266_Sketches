// ESP8266 TCP JSON Chat - Client
// Heavily commented version: explains what each block/line does, why it's here,
// potential pitfalls, and improvement suggestions.
//
// Purpose:
// - Connect the ESP8266 to Wi-Fi as a station.
// - Connect as a TCP client to the server device (ESP8266 server example).
// - Send/receive newline-delimited JSON messages over TCP.
// - Allow operator to type messages via Serial Monitor and send them as JSON.

// --------- Libraries ---------
#include <ESP8266WiFi.h>   // Core WiFi support for ESP8266
#include <ArduinoJson.h>   // JSON parsing/serialization library

// --------- WiFi credentials (replace before use) ---------
// NOTE: For production don't hardcode credentials; use secure provisioning.
const char* ssid     = "YOUR_SSID";   // WiFi network name (replace)
const char* password = "YOUR_PASS";   // WiFi password (replace)

// --------- Server connection info ---------
// Set this to the server device's IP (printed by the server).
// Example: IPAddress serverIP(192,168,1,123);
IPAddress serverIP(192,168,1,123); // <= CHANGE to server's IP
const uint16_t PORT = 3333;        // Port must match server's listening port

// --------- Networking and I/O state ---------
WiFiClient client;                  // WiFiClient object for the TCP connection
String inputBuffer = "";            // Buffer for characters typed into Serial until Enter
unsigned long lastReconnectAttempt = 0; // Track last attempt to reconnect
const unsigned long RECONNECT_INTERVAL = 5000; // ms between reconnect attempts

// -------------------- setup() --------------------
void setup() {
  Serial.begin(9600);             // Start Serial for debug & operator input
  delay(200);                       // Brief pause to let Serial initialize
  Serial.println();
  Serial.println("ESP8266 Chat Client starting...");

  // Put module in station mode (connect to an existing access point)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);       // Start connecting to WiFi

  Serial.printf("Connecting to %s ...\n", ssid);
  // Blocking wait until WiFi connects.
  // For more robust apps, use a non-blocking approach + timeout.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());   // Print the acquired IP address

  // Try to connect to the TCP server immediately after WiFi is up
  tryConnect();

  Serial.println("Type messages in Serial and press Enter to send JSON to server.");
}

// -------------------- loop() --------------------
void loop() {
  // If not connected to the server, periodically attempt reconnection.
  // This avoids constant tight-loop reconnection attempts.
  if (!client || !client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = now;
      tryConnect();
    }
  }

  // Read incoming JSON messages from the server.
  // Each message is expected to be newline-delimited '\n'.
  if (client && client.connected() && client.available()) {
    // readStringUntil('\n') reads until newline or timeout.
    // Note: It's simple but may block briefly if newline does not arrive.
    String line = client.readStringUntil('\n');
    line.trim();                     // remove trailing CR/LF and whitespace
    if (line.length() > 0) {
      handleIncomingJson(line);      // parse and display the JSON message
    }
  }

  // Read operator input from Serial monitor to send messages.
  // Characters accumulate in inputBuffer until Enter (newline).
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') { /* ignore CR on some terminals */ }
    else if (c == '\n') {
      // Enter pressed: send the buffered message if non-empty
      inputBuffer.trim();
      if (inputBuffer.length() > 0 && client && client.connected()) {
        sendJsonMessage("client", inputBuffer);
      } else if (inputBuffer.length() > 0) {
        Serial.println("Not connected to server. Message not sent.");
      }
      inputBuffer = "";             // reset buffer for next message
    } else {
      inputBuffer += c;             // append character to buffer
    }
  }
}

// -------------------- tryConnect() --------------------
// Attempt to connect to the serverIP on PORT. Does not block indefinitely.
void tryConnect() {
  // If we already have a connection, nothing to do.
  if (client && client.connected()) return;

  Serial.printf("Connecting to server %s:%u ...\n", serverIP.toString().c_str(), PORT);

  // client.connect returns true when TCP handshake succeeds.
  // This is a blocking attempt but typically reasonably quick.
  if (client.connect(serverIP, PORT)) {
    Serial.println("Connected to server.");
    // Optionally setNoDelay(true) to reduce latency as in the server:
    // client.setNoDelay(true);
  } else {
    Serial.println("Connection failed.");
    // The loop() will schedule another attempt after RECONNECT_INTERVAL.
  }
}

// -------------------- handleIncomingJson() --------------------
// Parse a newline-terminated JSON string from the server and print friendly info.
void handleIncomingJson(const String &jsonStr) {
  // Allocate a static JSON buffer on the stack.
  // Increase if messages include larger payloads or many fields.
  StaticJsonDocument<256> doc;

  DeserializationError err = deserializeJson(doc, jsonStr);
  if (err) {
    // Print parse error & raw JSON for debugging
    Serial.print("Failed to parse JSON: ");
    Serial.println(err.c_str());
    Serial.print("Raw: ");
    Serial.println(jsonStr);
    return;
  }

  // Extract fields with safe defaults if missing
  const char* from = doc["from"] | "unknown";
  const char* msg  = doc["message"] | "";
  unsigned long ts = doc["timestamp"] | 0;

  // Print a readable incoming message line
  // NOTE: timestamp is the sender's millis() value, not a wall-clock time.
  Serial.printf("[IN] %s (%lu): %s\n", from, ts, msg);
}

// -------------------- sendJsonMessage() --------------------
// Compose a JSON object { from, message, timestamp } and send over TCP.
// Adds a newline '\n' as a delimiter so receiver can split messages.
void sendJsonMessage(const char* sender, const String &message) {
  // If not connected, bail out silently (optional: return an error).
  if (!client || !client.connected()) return;

  // Prepare JSON document (stack-allocated)
  StaticJsonDocument<256> doc;
  doc["from"] = sender;
  doc["message"] = message;
  // Use millis() for a lightweight sender-side timestamp.
  // For synchronized time, integrate NTP and send epoch seconds.
  doc["timestamp"] = millis();

  // Serialize JSON to a String to send over the TCP connection
  String out;
  serializeJson(doc, out);
  out += "\n"; // newline delimiter required by receiver
  client.print(out); // send the JSON string

  // Echo the outgoing message to Serial for feedback
  Serial.printf("[OUT] %s: %s\n", sender, message.c_str());
}
