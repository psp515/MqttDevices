#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "JsonConfiguration.h"
#include "Seriallogger.h"
#include "MqttTransport.h"

using namespace smartdevices::configuration;
using namespace smartdevices::logging;
using namespace smartdevices::transport;

const char* filename = "appsettings.json";

SerialLogger* logger;
JsonConfiguration* configuration;
MqttTransport* transport;

bool wifiInitialized = false;
bool transportStarted = false;
unsigned long lastSend = 0;

void diag() {
  logger->info("--- System Info --- CPU Freq: %d, Heap: %d ", rp2040.f_cpu(), rp2040.getFreeHeap());
}

void listFiles(String dir_path) {
  Dir dir = LittleFS.openDir(dir_path);
	while(dir.next()) {
		if (dir.isFile()) {
      logger->info("--- File: %s ", dir_path + dir.fileName());
		}
		if (dir.isDirectory()) {
      logger->info("--- Dir: %s ", dir_path + dir.fileName() + "/");
		}
	}
}

void setup() {
  delay(3000);
  Serial.begin(115200);
  while(!Serial);

  logger = new SerialLogger(Serial, LogLevel::DEBUG);

  logger->info("----------- Configuration Init ----------- ");

  listFiles("/");

  configuration = new JsonConfiguration(*logger, filename);
  bool configStatus = configuration->load();

  if (!configStatus) {
    logger->error("Failed to initialize configuration.");
    delay(5000);
    return;
  }

  logger->info("----------- Transport Init ----------- ");
  transport = new MqttTransport(*configuration, *logger);

  logger->info("----------- Classes initialization successfull ----------- ");
}

void setClock()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void loop() {
  logger->info("----------- Wifi Init ----------- ");
  diag();

  logger->info("----------- Transport Init ----------- ");
  if (!transportStarted) {

    if (!transport->start()) {
      logger->error("Failed to start MQTT transport.");
      auto client = transport->getClient();
      logger->info("Is connected %d", client.connected());
      logger->info("MQTT state: %d", client.state());

      delay(5000);
      return;
    }

    transportStarted = true;
    logger->info("MQTT transport started.");
  }

  if (!wifiInitialized) {

    char ssid[64];
    char password[64];

    if (!configuration->getString("wifi:ssid", ssid, sizeof(ssid))) {
      logger->error("WiFi SSID missing in configuration.");
      delay(5000);
      return;
    }

    if (!configuration->getString("wifi:password", password, sizeof(password))) {
      logger->error("WiFi password missing in configuration.");
      delay(5000);
      return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    logger->info("Connecting to WiFi...");

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
      delay(500);
      Serial.print(".");
      retry++;
    }
    Serial.println(".");

    if (WiFi.status() == WL_CONNECTED) {
      logger->info("WiFi connected.");
      logger->info("Local ip: %s", WiFi.localIP());
      setClock();
      wifiInitialized = true;
    } else {
      logger->error("WiFi connection failed.");
      delay(5000);
      return;
    }
  }

  logger->info("----------- Transport Loop ----------- ");
  if (transportStarted) {

    bool status = transport->loop();

    if (!status) {
      logger->warn("MQTT transport not working.");
      delay(500);
      return;
    }

  }

  logger->info("----------- Transport Messaging ----------- ");
  if (transportStarted && millis() - lastSend > 10000) {

    char baseTopic[128];
    if (!configuration->getString("mqtt:baseTopic", baseTopic, sizeof(baseTopic))) {
      logger->error("Base topic missing.");
      return;
    }

    char fullTopic[256];
    snprintf(fullTopic, sizeof(fullTopic), "%s/status", baseTopic);

    TransportMessage msg(fullTopic, "device-alive");

    logger->info("Before messge sent.");
    // TODO: Debug error here

    if (transport->send(msg)) {
      logger->info("Message sent.");
    } else {
      logger->error("Failed to send message.");
    }

    lastSend = millis();
  }

  logger->info("Sleeping ...");  
  delay(1000);
}
