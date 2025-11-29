// ESP8266 TCP JSON Chat - Server
// Heavily commented version explaining each line, design choice and pitfalls.
//
// Purpose:
// - Create a simple TCP server on an ESP8266 that accepts one client connection.
// - Exchange JSON messages (one JSON object per line) between Serial (server operator)
//   and the connected TCP client.
// - Incoming JSON is parsed and printed to Serial; outgoing messages are wrapped into JSON
//   and sent to the client with a newline as a delimiter.

#include <ESP8266WiFi.h>   // Core WiFi library for ESP8266
#include <ArduinoJson.h>   // JSON parsing / serialization library

// NOTE: For production avoid hardcoding credentials in source code.
// Move credentials to secure storage or use build-time secrets.
const char* ssid     = "YOUR_SSID";   // WiFi network name (replace before deploying)
const char* password = "YOUR_PASS";   // WiFi password (replace before deploying)

const uint16_t PORT  = 3333;          // TCP port the server will listen on

WiFiServer server(PORT);              // WiFiServer object bound to defined PORT
WiFiClient client;                    // Single WiFiClient object to hold the connected client

// Buffer to accumulate characters typed into the Serial monitor until Enter pressed.
// Using String for simplicity — on memory-constrained devices be mindful of fragmentation.
String inputBuffer = "";

// -------------------- setup() --------------------
void setup() {
  Serial.begin(9600);               // Start Serial at 115200 baud for debugging and input
  delay(200);                         // Short delay to let serial come up
  Serial.println();
  Serial.println("ESP8266 Chat Server starting...");

  // Put WiFi into station mode (connect to an access point)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);         // Start connecting to the WiFi network

  Serial.printf("Connecting to %s ...\n", ssid);
  // Wait until we are connected. This is blocking — acceptable here for a simple sketch.
  // In more complex apps, you might want a non-blocking connect attempt with a timeout.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");                // Visual feedback while waiting
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());     // Print the acquired IP address

  server.begin();                     // Start listening for incoming TCP connections
  server.setNoDelay(true);            // Disable Nagle's algorithm: send packets immediately
  // setNoDelay(true) reduces latency for small frequent messages but can increase
  // packet overhead. Good for interactive chat-like apps.

  Serial.printf("Server listening on port %u\n", PORT);
  Serial.println("Type messages in Serial and press Enter to send JSON to client.");
}

// -------------------- loop() --------------------
void loop() {
  // Accept client if not connected
  // client.connected() returns true when the socket is open AND the client hasn't closed it.
  if (!client || !client.connected()) {
    // server.available() returns a WiFiClient representing a newly connected client (or empty client)
    client = server.available();
    if (client) {
      Serial.println("Client connected!");
      // Optional: you can print client.remoteIP() and client.remotePort() for more info:
      // Serial.printf("Remote: %s:%u\n", client.remoteIP().toString().c_str(), client.remotePort());
    }
  }

  // Read incoming data from client (JSON messages)
  // We expect that each JSON message from client ends with a newline '\n' delimiter
  if (client && client.connected() && client.available()) {
    // readStringUntil('\n') will collect bytes until newline or timeout.
    // It returns a String: ok for simple examples but watch memory fragmentation on long runs.
    String line = client.readStringUntil('\n');
    line.trim();                       // remove whitespace and CR/LF if present
    if (line.length() > 0) {
      // Process the received JSON line
      handleIncomingJson(line);
    }
  }

  // Read Serial input to send message to client
  // This allows a person using Serial Monitor to type messages and press Enter to send.
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') { /* ignore carriage return (for some terminals) */ }
    else if (c == '\n') {
      // When newline is detected we treat inputBuffer as a completed message
      inputBuffer.trim();
      if (inputBuffer.length() > 0 && client && client.connected()) {
        // Send JSON message with "server" as the sender
        sendJsonMessage("server", inputBuffer);
      } else if (inputBuffer.length() > 0) {
        // No client connected so notify operator — message was not delivered.
        Serial.println("No client connected. Message not sent.");
      }
      inputBuffer = "";               // Clear buffer for next message
    } else {
      // Accumulate characters into inputBuffer until Enter pressed
      inputBuffer += c;
    }
  }
}

// -------------------- handleIncomingJson() --------------------
// Parse JSON string received from client and print a readable line to Serial
void handleIncomingJson(const String &jsonStr) {
  // Size: 256 bytes allocated on the stack for JSON document.
  // This must be large enough to hold the parsed JSON object (keys + values).
  // If you expect larger messages, increase the size accordingly.
  StaticJsonDocument<256> doc;

  // Parse the JSON string. deserializeJson returns a DeserializationError (implicit bool).
  DeserializationError err = deserializeJson(doc, jsonStr);
  if (err) {
    // If parsing fails, print the error reason and the raw JSON to help debugging.
    Serial.print("Failed to parse JSON: ");
    Serial.println(err.c_str());     // e.g., "NoMemory", "InvalidInput", etc.
    Serial.print("Raw: ");
    Serial.println(jsonStr);
    return;
  }

  // Safely extract fields with default fallbacks:
  // - doc["from"] | "unknown" means: use "unknown" if the "from" field missing
  // - doc["message"] | "" means: empty string if missing
  const char* from = doc["from"] | "unknown";
  const char* msg  = doc["message"] | "";
  // timestamp: reading as unsigned long; default 0 if not present.
  unsigned long ts = doc["timestamp"] | 0;

  // Print a human readable incoming message on Serial
  // Note: ts is the millis() value sent by the sender (not a real Unix timestamp).
  Serial.printf("[IN] %s (%lu): %s\n", from, ts, msg);
}

// -------------------- sendJsonMessage() --------------------
// Create a JSON object with fields: from, message, timestamp and send to connected client.
// Adds a '\n' newline at the end so the receiver can split messages by newline.
void sendJsonMessage(const char* sender, const String &message) {
  // Safety: if no client or client disconnected — nothing to do.
  if (!client || !client.connected()) return;

  // Prepare JSON document (same size considerations as parse)
  StaticJsonDocument<256> doc;
  doc["from"] = sender;
  doc["message"] = message;
  // Use millis() as a lightweight timestamp (ms since boot). If you need real time,
  // integrate an RTC or NTP client and send epoch seconds instead.
  doc["timestamp"] = millis();

  // Serialize JSON document to a String to send over TCP
  String out;
  serializeJson(doc, out);
  out += "\n"; // newline acts as a message delimiter on the receiving side
  client.print(out); // send the JSON string to client

  // Echo outgoing message to Serial for operator feedback
  Serial.printf("[OUT] %s: %s\n", sender, message.c_str());
}
