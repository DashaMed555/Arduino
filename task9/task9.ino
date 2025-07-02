/*
 * Basic Sketch for automatic update of firmware at start
 * with firmware version control
 *
 * Renzo Mischianti <www.mischianti.org>
 *
 * https://mischianti.org
*/
 
#include <Arduino.h>
 
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
 
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <Arduino_JSON.h>
#include <SimpleDHT.h>

#include <PubSubClient.h>

#include <ESP8266WebServer.h>
#include <EEPROM.h>

int i = 0;
int statusCode;
const char* ssid = "IIR_WiFi";
const char* passphrase = "deeprobotics";
String st;
String content;

int mode = 0;
bool do_send = false;

// Конфигурация ThingSpeak
const char* thingspeak_api_key = "D1Q7STX8O2Z1RBWK";
const char* thingspeak_url = "http://api.thingspeak.com/update";

int max_temperature = 100;
int max_humidity = 100;

const int buttonPin = D3;
 
 
//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);

const char* mqtt_server = "192.168.68.76";
unsigned int port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

int DHTpin = D1;
SimpleDHT11 dht11(DHTpin);
 
ESP8266WiFiMulti WiFiMulti;
 
#define FIRMWARE_VERSION "0.1"
 
void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}
 
void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}
 
void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}
 
void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}
 
void setup() {
  Serial.println("clearing eeprom");
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();

  Serial.begin(115200);
//   Serial.setDebugOutput(true);

  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.println("Startup");
 
  //---------------------------------------- Read EEPROM for SSID and pass
  Serial.println("Reading EEPROM ssid");
 
  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");
 
  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
 
  Serial.println();
  Serial.println();
  Serial.println();
 
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
 
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(esid.c_str(), epass.c_str());
  
  WiFi.begin(esid.c_str(), epass.c_str());

  Serial.println();
  Serial.println("Waiting.");

  randomSeed(micros());

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
 
  Serial.print(F("Firmware version "));
  Serial.println(FIRMWARE_VERSION);
  delay(2000);

  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
}
 
 
void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
      // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED)) {
      // The line below is optional. It can be used to blink the LED on the board during flashing
      // The LED will be on during download of one buffer of data from the network. The LED will
      // be off during writing that buffer to flash
      // On a good connection the LED should flash regularly. On a bad connection the LED will be
      // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
      // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
      ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  
      // Add optional callback notifiers
      ESPhttpUpdate.onStart(update_started);
      ESPhttpUpdate.onEnd(update_finished);
      ESPhttpUpdate.onProgress(update_progress);
      ESPhttpUpdate.onError(update_error);
  
      ESPhttpUpdate.rebootOnUpdate(false); // remove automatic update
  
      Serial.println(F("Update start now!"));

      WiFiClient client_;
  
      // t_httpUpdate_return ret = ESPhttpUpdate.update(client, "http://192.168.1.70:3000/firmware/httpUpdateNew.bin");
      // Or:
      t_httpUpdate_return ret = ESPhttpUpdate.update(client_, "192.168.68.76", 3000, "/update", FIRMWARE_VERSION);
  
      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          Serial.println(F("Retry in 10secs!"));
          delay(10000); // Wait 10secs
          break;
  
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          Serial.println("Your code is up to date!");
            delay(10000); // Wait 10secs
          break;
  
        case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_OK");
          delay(1000); // Wait a second and restart
          ESP.restart();
          break;
      }
    }

    Serial.println("Measuring the values...");

    if (!client.connected()) {
      reconnect();
    }
    client.loop();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

    byte temperature;
    byte humidity;
    int err = SimpleDHTErrSuccess;
    
    if ((err = dht11.read(&temperature, &humidity, NULL)) == SimpleDHTErrSuccess) {
      JSONVar myObject;
      myObject["temperature"] = String((int)temperature) + String("ºC");
      myObject["humidity"] = String((int)humidity) + String("%");

      if ((int)temperature > max_temperature) {
        snprintf(msg, MSG_BUFFER_SIZE, "%s", "Too much!");
      }
      else {
        snprintf(msg, MSG_BUFFER_SIZE, "%s", String((int)temperature) + String("ºC"));
      }
      client.publish("user_d23c7cac/temperature", msg);

      if ((int)humidity > max_humidity) {
        snprintf(msg, MSG_BUFFER_SIZE, "%s", "Too much!");
      }
      else {
        snprintf(msg, MSG_BUFFER_SIZE, "%s", String((int)humidity) + String("%"));
      }
      client.publish("user_d23c7cac/humidity", msg);
      if (mode == 0) {
        sendToThingSpeak(temperature, humidity);
      }
      else if (do_send) {
        sendToThingSpeak(temperature, humidity);
        do_send = false;
      }
    } else {
      Serial.println("Error in reading the values!");
      Serial.println(err);
    }
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  bool currentButtonState = digitalRead(buttonPin);
  if (!currentButtonState) {
    setup();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("user_d23c7cac/max_temperature");
      client.subscribe("user_d23c7cac/max_humidity");
      client.subscribe("user_d23c7cac/mode");
      client.subscribe("user_d23c7cac/send");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {  // 111 means RED==ON, YELLOW==ON, GREEN==ON
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  Serial.println();

  if (String(topic) == "user_d23c7cac/max_temperature") {
    max_temperature = atoi(msg.c_str());
  }
  else if (String(topic) == "user_d23c7cac/max_humidity") {
    max_humidity = atoi(msg.c_str());
  }
  else if (String(topic) == "user_d23c7cac/mode") {
    if (msg.c_str()[0] == 0) {
      mode = 0;
    }
    else {
      mode = 1;
    }
  }
  else if (String(topic) == "user_d23c7cac/send") {
    do_send = true;
  }
}

//-------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change 
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}
 
void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}
 
void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
 
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("NeuroAgent", "123456789");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}
 
void createWebServer()
{
 {
    server.on("/", []() {
 
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>rn<html>Hello from ESP8266 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
 
      content = "<!DOCTYPE HTML>rn<html>go back";
      server.send(200, "text/html", content);
    });
 
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
 
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();
 
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);
 
    });
  } 
}

void sendToThingSpeak(int temperature, int humidity) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client_;
    HTTPClient http;
    
    String url = String(thingspeak_url) + "?api_key=" + thingspeak_api_key + 
                 "&field1=" + String(temperature) + 
                 "&field2=" + String(humidity);
    
    http.begin(client_, url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      Serial.printf("ThingSpeak response: %d\n", httpCode);
    } else {
      Serial.printf("ThingSpeak request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
  }
}
