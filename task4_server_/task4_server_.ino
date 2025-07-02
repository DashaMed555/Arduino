#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

const char* ssid = "IIR_WiFi";
const char* password = "deeprobotics";

int LED_R = 2;   // led connected to GPIO2 (D4)
int LED_Y = 4;   // led connected to GPIO2 (D2)
int LED_G = 5;   // led connected to GPIO2 (D1)

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/data", HTTP_POST, handlePOSTData);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}

void handlePOSTData() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.println("Received POST data: " + body);

    JSONVar doc = JSON.parse(body);

    digitalWrite(LED_R, int(doc["red"])); 
    digitalWrite(LED_Y, int(doc["yellow"])); 
    digitalWrite(LED_G, int(doc["green"])); 

    server.send(200, "application/json", "{\"status\":\"success\"}");
  } else {
    Serial.println("No POST data received");
    server.send(400, "text/plain", "No data received");
  }
}
