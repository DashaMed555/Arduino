#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "IIR_WiFi";
const char* password = "deeprobotics";
const char* mqtt_server = "srv2.clusterfly.ru";
const char* mqtt_username = "user_d23c7cac";
const char* mqtt_password = "B-6T7L9ZoSKbP";
unsigned int port = 9991;

WiFiClient espClient;
PubSubClient client(espClient);

int LED_R = 2;   // led connected to GPIO2 (D4)
int LED_Y = 4;   // led connected to GPIO2 (D2)
int LED_G = 5;   // led connected to GPIO2 (D1)

void callback(char* topic, byte* payload, unsigned int length) {  // 111 means RED==ON, YELLOW==ON, GREEN==ON
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_R, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(LED_R, LOW);  // Turn the LED off by making the voltage HIGH
  }

  if ((char)payload[1] == '1') {
    digitalWrite(LED_Y, HIGH);
  } else {
    digitalWrite(LED_Y, LOW);
  }

  if ((char)payload[2] == '1') {
    digitalWrite(LED_G, HIGH);
  } else {
    digitalWrite(LED_G, LOW);
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  randomSeed(micros());

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, port);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("state");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}
