#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <time.h>

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

unsigned long previousMillis = 0;
// bool buttonPressed = false;
bool lastButtonState = HIGH;


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

  // attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
}

// the loop function runs over and over again forever
void loop() {
  unsigned long currentMillis = millis();

  JSONVar colours;
  if (MODE) {
    if (currentMillis - previousMillis >= DURATIONS[STATE] * 1000) {
      previousMillis = currentMillis;
      colours = get_json(STATES1[STATE]);
      digitalWrite(LED_R, colours["red"]); 
      digitalWrite(LED_Y, colours["yellow"]); 
      digitalWrite(LED_G, colours["green"]);
      STATE = (STATE + 1) % 16;
    }
  }
  else {
    if (currentMillis - previousMillis >= DURATIONS_[PREV] * 1000) {
      previousMillis = currentMillis;
      colours = get_json(STATES_[PREV]);
      digitalWrite(LED_R, colours["red"]); 
      digitalWrite(LED_Y, colours["yellow"]); 
      digitalWrite(LED_G, colours["green"]);
      PREV = (PREV + 1) % 2;
    }
  }

  // if (buttonPressed) {
  //   MODE = !MODE;
  //   STATE = 0;
  //   PREV = 0;
  //   buttonPressed = false;
  // }

  sendRequest("http://192.168.68.79/data");

  bool currentButtonState = digitalRead(buttonPin);
  if (currentButtonState != lastButtonState) {
    MODE = !MODE;
    STATE = 0;
    PREV = 0;
    lastButtonState = currentButtonState;
  }

  // yield();
}

JSONVar get_json(COLOURS colour) {
  JSONVar colours;
  switch (colour) {
    case green: 
      colours["red"] = LOW;
      colours["yellow"] = LOW;
      colours["green"] = HIGH;
      return colours;
    case nothing:
      colours["red"] = LOW;
      colours["yellow"] = LOW;
      colours["green"] = LOW;
      return colours;
    case yellow:
      colours["red"] = LOW;
      colours["yellow"] = HIGH;
      colours["green"] = LOW;
      return colours;
    case red:
      colours["red"] = HIGH;
      colours["yellow"] = LOW;
      colours["green"] = LOW;
      return colours;
    case red_and_yellow:
      colours["red"] = HIGH;
      colours["yellow"] = HIGH;
      colours["green"] = LOW;
      return colours;
  }
  return colours;
}

void sendRequest(const char* url) {
  WiFiClient client;
  HTTPClient http;

  if (http.begin(client, url)) {
    JSONVar colours;
    if (MODE) {
      colours = get_json(STATES2[STATE]);
    }
    else {
      colours = get_json(STATES_[PREV]);
    }
    String jsonString = JSON.stringify(colours);

    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonString);
    if (httpCode > 0) {
      Serial.printf("HTTP POST request to %s succeeded, code: %d\n", url, httpCode);
      String payload = http.getString();
      Serial.println("Response payload: " + payload);
    } else {
      Serial.printf("HTTP POST request to %s failed, error: %s\n", url, http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }
}

// void buttonISR() {
//   buttonPressed = true;
// }
