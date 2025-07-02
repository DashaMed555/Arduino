#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "IIR_WiFi";
const char* password = "deeprobotics";

ESP8266WebServer server(80);

const int ledPin = D4; // Пин, к которому подключен светодиод
const int buttonPin = D3; // Пин, к которому подключена кнопка

bool ledState = LOW;
bool lastButtonState = HIGH;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/led/on", handleLedOn);
  server.on("/led/off", handleLedOff);

  server.begin();
}

void loop() {
  server.handleClient();

  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState != lastButtonState) {
    if (currentButtonState == LOW) {
      sendRequest("http://192.168.68.58/led/on");
    } else {
      sendRequest("http://192.168.68.58/led/off");
    }
    lastButtonState = currentButtonState;
  }
}

void handleRoot() {
  server.send(200, "text/plain", "Hello from NodeMCU!");
}

void handleLedOff() {
  digitalWrite(ledPin, HIGH);
  ledState = HIGH;
  server.send(200, "text/plain", "LED is ON");
}

void handleLedOn() {
  digitalWrite(ledPin, LOW);
  ledState = LOW;
  server.send(200, "text/plain", "LED is OFF");
}

void sendRequest(const char* url) {
  WiFiClient client;
  HTTPClient http;

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("HTTP GET request to %s succeeded, code: %d\n", url, httpCode);
    } else {
      Serial.printf("HTTP GET request to %s failed, error: %s\n", url, http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }
}