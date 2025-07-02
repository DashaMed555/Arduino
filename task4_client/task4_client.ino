#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

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
}

void loop() {
  // put your main code here, to run repeatedly:
  sendRequest("http://192.168.68.58/data");
  delay(100);
}

void sendRequest(const char* url) {
  WiFiClient client;
  HTTPClient http;

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("HTTP GET request to %s succeeded, code: %d\n", url, httpCode);
      String payload = http.getString();
      JSONVar doc = JSON.parse(payload);

      digitalWrite(LED_R, int(doc["red"])); 
      digitalWrite(LED_Y, int(doc["yellow"])); 
      digitalWrite(LED_G, int(doc["green"])); 
    } else {
      Serial.printf("HTTP GET request to %s failed, error: %s\n", url, http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }
}
