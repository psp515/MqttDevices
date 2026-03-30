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
#include "ClockService.h"

// ------- Implementations
#include "SerialLogger.h"
#include "JsonConfiguration.h"
#include "PicoWifiManager.h"
#include "SecureMqttTransport.h"
#include "NtpClockService.h"

#define DHTPIN 14  
#define DHTTYPE DHT22  

#define HUMAN_PRESENCE_RX_Pin 5
#define HUMAN_PRESENCE_TX_Pin 4
#define HUMAN_PRESENCE_Pin 15

using namespace smartdevices::logging;
using namespace smartdevices::configuration;
using namespace smartdevices::network;
using namespace smartdevices::transport;
using namespace smartdevices::clock;

// ------- Device Agnostic
//SerialLogger serialLogger(Serial, LogLevel::DEBUG);
SerialLogger serialLogger(Serial);
JsonConfiguration jsonConfiguration(serialLogger, "./appsettings.json");
PicoWifiManager wifiManagerRp(jsonConfiguration, serialLogger);
SecureMqttTransport secureTransport(jsonConfiguration, serialLogger);
NtpClockService nptTimeService(jsonConfiguration, serialLogger);

SoftwareSerial mySerial = SoftwareSerial(HUMAN_PRESENCE_RX_Pin, HUMAN_PRESENCE_TX_Pin);
HumanStaticLite radar = HumanStaticLite(&mySerial);
DHT dht(DHTPIN, DHTTYPE);

Logger& logger = serialLogger;
Configuration& configuration = jsonConfiguration;
WifiManager& wifi = wifiManagerRp;
Transport& transport = secureTransport;
ClockService& timeService = nptTimeService;

// ------- Device Specific
unsigned long tempLastRead = 0;  
unsigned long tempLastSend = 0;   
float lastTemp = NAN;
const unsigned long tempReadInterval = 30 * 1000; 
const unsigned long tempSendInterval = 5*60*1000; 
const float tempThreshold = 0.5;           

unsigned long presenceLastSend = 0;
int lastHumanPresenceState = 0;
const unsigned long presenceInterval = 5000;      

int timeout = 100;
unsigned long lastSysLog = 0;
bool _wifiInProgressLogged = false;
bool _timeSyncInProgressLogged = false;
bool _transportInProgressLogged = false;

bool _wifiDisconnected = true;

bool presenceUpdatesEnabled = true;
bool forcePresenceUpdate = false;
bool temperatureUpdatesEnabled = true; 
bool forceTemperatureUpdate = false;

void presenceStateUpdates(const TransportMessage& message) {
    std::string payload = message.getPayload();
    bool presence = (payload == "true");

    if (presence == presenceUpdatesEnabled) {
      logger.warn("[Device] Presence update ignored.");
      return;
    }
    
    presenceUpdatesEnabled = presence;

    if (presence) {
      forcePresenceUpdate = true;
    }

    logger.info("[Device] Presence updates enabled set to: %s", presenceUpdatesEnabled ? "true" : "false");
}

void temperatureStateUpdates(const TransportMessage& message) {
    std::string payload = message.getPayload();
    bool temperatureState = (payload == "true");
  
    if (temperatureState == temperatureUpdatesEnabled) {
      logger.warn("[Device] Temperature update ignored.");
      return;
    }

    temperatureUpdatesEnabled = temperatureState;

    if (temperatureState) {
      forceTemperatureUpdate = true;
    }

    logger.info("[Device] Temperature updates enabled set to: %s", temperatureUpdatesEnabled ? "true" : "false");
}

void observeEvents() {
  transport.observe("environment/status", temperatureStateUpdates);
  transport.observe("presence/status", presenceStateUpdates);
}

void setup() {
  delay(2000);
  Serial.begin(115200);
  mySerial.begin(115200);
  while(!Serial);

  if (!LittleFS.begin()) {
    logger.error("Failed to start filesystem. Tools > Flash Size > at least 64KB");
    delay(15000);
    watchdog_reboot(0, 0, 0);
  }

  logger.info("--------- Initializing");
  pinMode(HUMAN_PRESENCE_Pin, INPUT); 
  dht.begin();
  radar.HumanStatic_func(false);
  configuration.load();

  if (!wifi.setup()) {
    logger.error("Failed to initialize wifi client. Fix configuration.");
    delay(15000);
    watchdog_reboot(0, 0, 0);
  }

  if (!timeService.setup()) {
    logger.error("Failed to initialize ntp. Fix configuration.");
    delay(15000);
    watchdog_reboot(0, 0, 0);
  }

  if (!transport.setup()) {
    logger.error("Failed to initialize transport. Fix configuration.");
    delay(15000);
    watchdog_reboot(0, 0, 0);
  }

  observeEvents();
  watchdog_enable(5000, 1);
}

void loop() {
  unsigned long now = millis();
  watchdog_update();

  if (now - lastSysLog >= 30000) {
    lastSysLog = now;
    logger.info("--- System Info, CPU Freq: %d, Heap: %d ", rp2040.f_cpu(), rp2040.getFreeHeap());
  }

  //--------- Monitor WiFi
  bool wifiStatus = wifi.loop();

  if (!wifiStatus) {
    if (!_wifiInProgressLogged) {
      logger.warn("Wifi connection in progress...");
      _wifiInProgressLogged = true;
    }

    _wifiDisconnected = true;;
    return;
  } else {
    _wifiInProgressLogged = false;
  }

  //--------- Monitor Tiome
  bool timeStatus = timeService.loop(); 
  // false is returned only on initialization

  if (!timeStatus) {
    if (!_timeSyncInProgressLogged) {
      logger.warn("Time synchronization in progress...");
      _timeSyncInProgressLogged = true;
    }

    return;
  } else {
    _timeSyncInProgressLogged = false;
  }

  //--------- Monitor MQTT
  bool transportStatus = transport.loop();

  if (!transportStatus) {
    if (!_transportInProgressLogged) {
      logger.warn("Transport connection in progress...");
      _transportInProgressLogged = true;
    }

    return;
  } else {
    _transportInProgressLogged = false;
  }

  //--------- Device functions

  if (_wifiDisconnected) {
    logger.info("Device connected, sending state update.");

    smartdevices::transport::MqttTransportMessage msg(
        "status",
        "true",
        true
    );

    bool sendStatus = transport.send(msg);

    smartdevices::transport::MqttTransportMessage temperatureMsg(
      "environment/status",
      (temperatureStateUpdates ? "true" : "false")
    );

    transport.send(temperatureMsg);

    smartdevices::transport::MqttTransportMessage presenceMsg(
      "presence/status",
      (presenceUpdatesEnabled ? "true" : "false")
    );

    transport.send(presenceMsg);

    if (!sendStatus) {
      logger.error("Failed to send status update after WiFi reconnect.");
    } else {
      _wifiDisconnected = false;
    }
  }

  //--------- Monitor DHT22
  if ((now - tempLastRead >= tempReadInterval && temperatureUpdatesEnabled) || forceTemperatureUpdate) {

    tempLastRead = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(t) && !isnan(h) && ((!lastTemp || abs(t - lastTemp) >= tempThreshold || now - tempLastSend >= tempSendInterval) || forceTemperatureUpdate)) {
      tempLastSend = now;
      lastTemp = t;
      forceTemperatureUpdate = false;

      logger.info("Temp: %.1f °C  Hum: %.1f %", t, h);

      char payload[128];
      snprintf(payload, sizeof(payload), "{\"temp\": %.1f, \"tempUnit\": \"°C\", \"hum\": %.1f, \"humUnit\": \"%%\"}", t, h);
      MqttTransportMessage msg(
          "environment",
          reinterpret_cast<const uint8_t*>(payload),
          strlen(payload)
      );

      bool sendStatus = transport.send(msg);

      if (!sendStatus) {
        logger.error("Failed to send temp and hum update.");
      }
    }
  }

  //--------- Monitor Static Presence
  int humanPresenceState = digitalRead(HUMAN_PRESENCE_Pin);

  if ((humanPresenceState != lastHumanPresenceState && presenceUpdatesEnabled) || forcePresenceUpdate) {
    if ((now - presenceLastSend >= 2500) || forcePresenceUpdate) {
      forcePresenceUpdate = false;
      lastHumanPresenceState = humanPresenceState;
      presenceLastSend = now;

      logger.info(humanPresenceState == HIGH ? "Human present." : "No human present.");

      smartdevices::transport::MqttTransportMessage msg(
          "presence",
          (humanPresenceState == HIGH ? "true" : "false")
      );

      bool sendStatus = transport.send(msg);

      if (!sendStatus) {
          logger.error("Failed to send presence update.");
      }
    } 

  }

  delay(timeout);
}