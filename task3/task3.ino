#include <SimpleDHT.h>
#include <Arduino_JSON.h>
#include <ESP8266WebServer.h>
int DHTpin = 2;

const char* ssid = "IIR_WiFi";
const char* password = "deeprobotics";

ESP8266WebServer server(80);

SimpleDHT11 dht11(DHTpin);
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/data", HTTP_GET, handleGetData);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleGetData() {
  byte temperature;
  byte humidity;
  int err = SimpleDHTErrSuccess;
  
  if ((err = dht11.read(&temperature, &humidity, NULL)) == SimpleDHTErrSuccess) {
    JSONVar myObject;
    myObject["temperature"] = String((int)temperature) + String("ºC");
    myObject["humidity"] = String((int)humidity) + String("%");
    String jsonString = JSON.stringify(myObject);
    server.send(200, "application/json", jsonString);
  } else {
    server.send(500, "application/json", "{\"error\":\"Failed.\"}");
  }
}
