#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "JsonConfiguration.h"
#include "SerialLogger.h"
#include "MqttTransport.h"

using namespace smartdevices::configuration;
using namespace smartdevices::logging;
using namespace smartdevices::transport;

const char* filename = "/appsettings.json";

void listFiles(String dir_path) {
  Dir dir = LittleFS.openDir(dir_path);
	while(dir.next()) {
		if (dir.isFile()) {
			// print file names
			Serial.print("File: ");
			Serial.println(dir_path + dir.fileName());
		}
		if (dir.isDirectory()) {
			// print directory names
			Serial.print("Dir: ");
			Serial.println(dir_path + dir.fileName() + "/");
		}
	}
}

SerialLogger logger(Serial, LogLevel::DEBUG);
JsonConfiguration config(logger, filename);
MqttTransport transport(config, logger);

bool wifiInitialized = false;
bool transportStarted = false;
unsigned long lastSend = 0;

void diag() {
  Serial.println("---- SYSTEM INFO ----");
  Serial.print("CPU Frequency: ");
  Serial.println(rp2040.f_cpu());

  Serial.print("Free Heap: ");
  Serial.println(rp2040.getFreeHeap());

  Serial.print("Core: ");
  Serial.println(get_core_num());
}

void setup() {
  Serial.begin(115200);
  while(!Serial);

  Serial.println("Working ...");   

  logger.info("----------- List files ----------- ");
  listFiles("./");

  logger.info("----------- Configuration Init ----------- ");
  bool configStatus = config.load();

  if (!configStatus) {
    logger.error("Failed to initialize configuration.");
    delay(5000);
    return;
  }

  logger.info("Config loaded.");
  diag();
}

void loop() {
  logger.info("----------- Wifi Init ----------- ");
  diag();

  if (!wifiInitialized) {

    char ssid[64];
    char password[64];

    if (!config.getString("wifi:ssid", ssid, sizeof(ssid))) {
      logger.error("WiFi SSID missing in configuration.");
      delay(5000);
      return;
    }

    if (!config.getString("wifi:password", password, sizeof(password))) {
      logger.error("WiFi password missing in configuration.");
      delay(5000);
      return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    logger.info("Connecting to WiFi...");

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
      delay(500);
      Serial.print(".");
      retry++;
    }
    Serial.println(".");

    if (WiFi.status() == WL_CONNECTED) {
      logger.info("WiFi connected.");
      wifiInitialized = true;
    } else {
      logger.error("WiFi connection failed.");
      delay(5000);
      return;
    }
  }

  logger.info("----------- Transport Init ----------- ");
  diag();

  if (!transportStarted) {

    if (!transport->start()) {
      logger.error("Failed to start MQTT transport.");
      auto client = transport->getClient();
      logger.info("Is connected %d", client.connected());
      logger.info("MQTT state: %d", client.state());

      delay(5000);
      return;
    }

    transportStarted = true;
    logger.info("MQTT transport started.");
  }

  logger.info("----------- Transport Messaging ----------- ");
  diag();
  if (transportStarted && millis() - lastSend > 10000) {

    char baseTopic[128];
    if (!config.getString("mqtt:baseTopic", baseTopic, sizeof(baseTopic))) {
      logger.error("Base topic missing.");
      return;
    }

    char fullTopic[256];
    snprintf(fullTopic, sizeof(fullTopic), "%s/status", baseTopic);

    TransportMessage msg(fullTopic, "device-alive");

    logger.info("Before messge sent.");
    // TODO: Debug error here

    if (transport->send(msg)) {
      logger.info("Message sent.");
    } else {
      logger.error("Failed to send message.");
    }

    lastSend = millis();
  }

  logger.info("----------- Transport Loop ----------- ");
  diag();
  if (transportStarted) {

    transport->loop();

  }

  logger.info("Sleeping ...");  
  delay(1000);
}
