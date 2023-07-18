#include <WiFi.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin("ASUS", "vikommeretterdeg");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }

  Serial.println("Connected to Wi-Fi!");
  Serial.println("Starting server...");
  server.on("/", handleRequest);
  server.begin();
  Serial.print("Server started! IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Empty loop
}

void handleRequest(AsyncWebServerRequest *request) {
  Serial.println("Received request!");

  Serial.println("Scanning for devices on the network...");
  WiFi.scanNetworks(true);

  int devicesFound = WiFi.scanComplete();

  if (devicesFound == WIFI_SCAN_FAILED) {
    Serial.println("Device scan failed.");
  } else if (devicesFound == 0) {
    Serial.println("No devices found on the network.");
  } else {
    Serial.println("Devices found on the network:");

    for (int i = 0; i < devicesFound; ++i) {
      String mac = WiFi.BSSIDstr(i);

      Serial.print("Device MAC Address: ");
      Serial.println(mac);
    }
  }

  request->send(200, "text/plain", "Device scan complete.");
}
