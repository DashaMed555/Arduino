#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>

ESP8266WebServer server(80);

const char* ssid = "IIR_WiFi";
const char* password = "deeprobotics";

const int buttonPin = D3;

int LED_R = 2;   // led connected to GPIO2 (D4)
int LED_Y = 4;   // led connected to GPIO2 (D2)
int LED_G = 5;   // led connected to GPIO2 (D1)

enum COLOURS 
{
  green,
  nothing,
  yellow,
  red,
  red_and_yellow
};

bool MODE = true;

int STATE = 0;

COLOURS STATES1[16] = {
  green, 
  nothing, 
  green, 
  nothing, 
  green, 
  nothing, 
  green, 
  yellow,
  red,
  red,
  red,
  red,
  red,
  red,
  red,
  red_and_yellow
};

COLOURS STATES2[16] = {
  red,
  red,
  red,
  red,
  red,
  red,
  red,
  red_and_yellow,
  green, 
  nothing, 
  green, 
  nothing, 
  green, 
  nothing, 
  green, 
  yellow
};

int DURATIONS[16] = {6, 1, 1, 1, 1, 1, 1, 3, 6, 1, 1, 1, 1, 1, 1, 3};

bool PREV = 0;

COLOURS STATES_[2] = {
  yellow, 
  nothing
};

int DURATIONS_[2] = {1, 1};

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);

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

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
}

// the loop function runs over and over again forever
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.begin(ssid, password);
    delay(1000);
  }
  server.handleClient();
  JSONVar colours;
  if (MODE) {
    colours = get_json(STATES1[STATE]);
    digitalWrite(LED_R, colours["red"]); 
    digitalWrite(LED_Y, colours["yellow"]); 
    digitalWrite(LED_G, colours["green"]); 
    delay(DURATIONS[STATE] * 1000);
    STATE = (STATE + 1) % 16;
  }
  else {
    colours = get_json(STATES_[PREV]);
    digitalWrite(LED_R, colours["red"]); 
    digitalWrite(LED_Y, colours["yellow"]); 
    digitalWrite(LED_G, colours["green"]); 
    delay(DURATIONS_[PREV] * 1000);
    PREV = (PREV + 1) % 2;
  }
}

JSONVar get_json(COLOURS colour) {
  JSONVar colours;
  switch (colour) {
    case green: 
      colours["red"] = 0;
      colours["yellow"] = 0;
      colours["green"] = 1;
      return colours;
    case nothing:
      colours["red"] = 0;
      colours["yellow"] = 0;
      colours["green"] = 0;
      return colours;
    case yellow:
      colours["red"] = 0;
      colours["yellow"] = 1;
      colours["green"] = 0;
      return colours;
    case red:
      colours["red"] = 1;
      colours["yellow"] = 0;
      colours["green"] = 0;
      return colours;
    case red_and_yellow:
      colours["red"] = 1;
      colours["yellow"] = 1;
      colours["green"] = 0;
      return colours;
  }
  return colours;
}

void handleGetData() {
  JSONVar colours;
  if (MODE) {
    colours = get_json(STATES2[STATE]);
  }
  else {
    colours = get_json(STATES_[PREV]);
  }
  String jsonString = JSON.stringify(colours);
  server.send(200, "application/json", jsonString);
}

void buttonISR() {
  MODE = not MODE;
  STATE = 0;
  PREV = 0;
}
