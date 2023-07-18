#include <WiFi.h>
#include <ESP32Ping.h>

const char* ssid = "MinecraftServer";
const char* password = "vikommeretterdeg";
IPAddress ipAddress(192, 168, 1, 48); // IP address to ping

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (Ping.ping("Vulcan.net")) {
    Serial.println("Ping successful");
  } else {
    Serial.println("Ping failed");
  }

  delay(5000); // Ping every 5 seconds
}
