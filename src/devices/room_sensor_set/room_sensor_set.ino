#include <Arduino.h>
#include <humanstaticLite.h>
#include <SoftwareSerial.h>
#include <String.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "hardware/watchdog.h"

// ------- Interfaces
#include "Logger.h"
#include "Configuration.h"
#include "WifiManager.h"
#include "Transport.h"

// ------- Implementations
#include "SerialLogger.h"
#include "JsonConfiguration.h"
#include "PicoWifiManager.h"
#include "SecureMqttTransport.h"

#define DHTPIN 14  
#define DHTTYPE DHT22  

#define HUMAN_PRESENCE_RX_Pin 5
#define HUMAN_PRESENCE_TX_Pin 4
#define HUMAN_PRESENCE_Pin 15

using namespace smartdevices::logging;
using namespace smartdevices::configuration;
using namespace smartdevices::network;
using namespace smartdevices::transport;

SerialLogger serialLogger(Serial);
JsonConfiguration jsonConfiguration(serialLogger, "./appsettings.json");
PicoWifiManager wifiManagerRp(jsonConfiguration, serialLogger);
SecureMqttTransport secureTransport(jsonConfiguration, serialLogger);

SoftwareSerial mySerial = SoftwareSerial(HUMAN_PRESENCE_RX_Pin, HUMAN_PRESENCE_TX_Pin);
HumanStaticLite radar = HumanStaticLite(&mySerial);
DHT dht(DHTPIN, DHTTYPE);

Logger& logger = serialLogger;
Configuration& configuration = jsonConfiguration;
WifiManager& wifi = wifiManagerRp;
Transport& transport = secureTransport;

bool setupClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  logger.info("Syncing NTP time.");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  logger.info("NTP time: %s", asctime(&timeinfo));

  return true;
}

void setup() {
  // --------- Setup
  delay(5000);
  Serial.begin(115200);
  mySerial.begin(115200);
  while(!Serial);

  if (!LittleFS.begin()) {
    logger.error("Failed to start filesystem");
    delay(5000);
    watchdog_reboot(0, 0, 0);
  }

  // --------- Initialize
  logger.info("--------- Initializing");
  pinMode(HUMAN_PRESENCE_Pin, INPUT); 
  dht.begin();
  radar.HumanStatic_func(false);
  configuration.load();

  if (!wifi.setup()) {
    logger.error("Failed to initialize wifi client.");
    delay(5000);
  }

  if (!setupClock()) {
    logger.error("Failed to connect with wifi.");
    delay(5000);
    watchdog_reboot(0, 0, 0);
  }

  if (!transport.setup()) {
    logger.error("Failed to initialize transport.");
    delay(5000);
  }

  logger.info("--------- Initialized");
}

int i = 0;
int timeout = 100;
static int lastButtonState = LOW; 

void loop() {

  //--------- Monitor WiFi
  bool wifiStatus = wifi.loop();
  if (!wifiStatus) {
    logger.warn("Wifi connection failed...");
    delay(500);
    return;
  }

  //--------- Monitor MQTT

  bool transportStatus = transport.loop();

  if (!transportStatus) {
    logger.warn("Transport connection failed...");
    delay(500);
    return;
  }

  //--------- Device functions
  i++;
  
  // if recconnected send update
  //--------- Monitor Device Heap
  if (i%100 == 0) {
    logger.info("--- System Info, CPU Freq: %d, Heap: %d ", rp2040.f_cpu(), rp2040.getFreeHeap());
  }

  //--------- Monitor MQTT
  if (i%250 == 0) {
    i = 0;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
      logger.error("Failed to read from DHT sensor!");
      return;
    }

    Serial.println(String("Temp: ") + dht.readTemperature() + "°C  Hum: " + dht.readHumidity() + "%");
  }

  //--------- Monitor Static Presence
  int buttonState = digitalRead(HUMAN_PRESENCE_Pin);

  if (buttonState != lastButtonState) {
      lastButtonState = buttonState;

      if (buttonState == HIGH) {
          logger.info("Human present.");
      } else {
          logger.info("No human present.");
      }
  }

  delay(timeout);
}