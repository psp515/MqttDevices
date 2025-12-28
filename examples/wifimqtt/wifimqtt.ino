#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFiClientSecureBearSSL.h>

/* TODO:
- configuration - 
- read trusted Ca certificatre
- read wifi configuration
  - ssid
  - password
- read mqtt configuration
  - broker
  - port
  - username
  - password
  - ca certificate / insecure
  - base topic
- device config
  - topic to send
  
- start read if button was pushed
- connect to wifi
- connect to mqtt
- read last send value (int)+1
- store n+1
- sened event - clicked
- deepsleep
*/

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("", "");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
}

void connectMQTT() {
  //espClient.setInsecure();

  espClient.

  mqttClient.setServer("", 8883);
  while (!mqttClient.connected()) {
    mqttClient.connect("", "", "");
    delay(1500);
    Serial.println("still not connected");
    Serial.println(mqttClient.state());
  }
}

void setup() {
  unsigned long startTime = millis(); // start timer
  Serial.begin(115200);
  int value = digitalRead(PIN); // HIGH or LOW
  delay(100);   
  Serial.println("Program starting");

  Serial.println(" starting");
  Serial.println(value);
  connectWiFi();
  Serial.println("wifi connected");

  connectMQTT();
  Serial.println("Connected");

  mqttClient.publish(
    "test",
    "test",
    true);
    
  unsigned long endTime = millis();
  Serial.print("Setup execution time: ");
  Serial.print(endTime - startTime);
  Serial.println(" ms");
  delay(100);       
}

void loop() {  
    
  Serial.println("Eternal sleep");
  ESP.deepSleep(0); 
}
