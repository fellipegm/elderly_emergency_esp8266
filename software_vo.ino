#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

#include <EEPROM.h>

const int end_eeprom = 0;

ESP8266WiFiMulti WiFiMulti;

int ledState = LOW;

unsigned long previousMillis = 0;
const long interval = 2000;

const int inputa = 5;
const int inputb = 4;

const int led_2 = 2;

String api_server = "http://api.pushingbox.com/pushingbox?devid="; 
String deviceId = "";  // your device ID from pushbox
const uint16_t port = 80;

// Wifi Configuration
#ifndef STASSID
#define STASSID "" // your wifi ssid
#define STAPSK  "" // your wifi password
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;


void setup() {
  Serial.begin(9600);

  EEPROM.begin(512);
  
  // Pin configurations
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(led_2, OUTPUT);
  pinMode(inputa, INPUT);
  pinMode(inputb, INPUT);
  
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(LED_BUILTIN, ledState);
  }
  
  int in1 = digitalRead(inputa);
  int in2 = digitalRead(inputb);

  if (EEPROM.read(end_eeprom)){
    digitalWrite(led_2, 0);
  }
  else {
    digitalWrite(led_2, 1);
  }

  if (in1 || in2 || EEPROM.read(end_eeprom)){
    digitalWrite(LED_BUILTIN, 0);
    digitalWrite(led_2, 0);
    Serial.println("Received signal");
    EEPROM.write(end_eeprom, 1);
    if (!EEPROM.commit()){
      Serial.println("EEPROM error, restarting");
      delay(30000);
      return;
    }
    
    WiFiClient client;
    HTTPClient http;
    
    if (WiFiMulti.run() == WL_CONNECTED) {
      if (http.begin(client, api_server + deviceId)) {  // HTTP
        Serial.println("Sending get request to:" + (api_server + deviceId));
        int httpCode = http.GET();
        if (httpCode > 0){
          Serial.println("Message transmitted with success");
        }
        else{
          Serial.println("Problem transmitting the message");
          delay(30000);
          return;
        }
        delay(30000);
        httpCode = http.GET();
        if (httpCode > 0){
          Serial.println("Message transmitted with success");
        }
        else{
          Serial.println("Problem transmitting the message");
          delay(30000);
          return;
        }
        EEPROM.write(end_eeprom, 0);
        EEPROM.commit();
        digitalWrite(LED_BUILTIN, 1);
        digitalWrite(led_2, 1);
        delay(120000); // Wait 2 min to send another message
      }
      else{
        Serial.println("Problem with pushbox");
        delay(30000);
        return;
      }
    }
    else {
      Serial.println("Could not connect to wifi");
      delay(30000);
      return;
    }
  }
}
