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

// ------- Implementations
#include "SerialLogger.h"
#include "JsonConfiguration.h"
#include "PicoWifiManager.h"

#define DHTPIN 14  
#define DHTTYPE DHT22  

#define HUMAN_PRESENCE_RX_Pin 5
#define HUMAN_PRESENCE_TX_Pin 4
#define HUMAN_PRESENCE_Pin 15

using namespace smartdevices::logging;
using namespace smartdevices::configuration;
using namespace smartdevices::network;

SoftwareSerial mySerial = SoftwareSerial(HUMAN_PRESENCE_RX_Pin, HUMAN_PRESENCE_TX_Pin);
HumanStaticLite radar = HumanStaticLite(&mySerial);

SerialLogger serialLogger(Serial);
JsonConfiguration jsonConfiguration(serialLogger, "./appsettings.json");
PicoWifiManager wifiManagerRp(jsonConfiguration, serialLogger);

Logger& logger = serialLogger;
Configuration& configuration = jsonConfiguration;
WifiManager& wifi = wifiManagerRp;

DHT dht(DHTPIN, DHTTYPE);

BearSSL::WiFiClientSecure wifiClientSecure;
PubSubClient client(wifiClientSecure);

// class SecureMqttTransport : public Transport {
// public:



// private:



// };


void callback(char* topic, byte* payload, unsigned int length) {

  logger.info("[Topic: %s] Message arrived.", topic);

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

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

bool setupCaCertificate() {
    logger.info("MQTT SSL - initializing CA certificate.");

    bool secureClientEnabled = false;
    configuration.getBool("mqtt:ssl:enabled", secureClientEnabled);

    if (!secureClientEnabled) {
        logger.warn("MQTT SSL - not enabled.");
        return true;
    }

    bool secure = false;
    configuration.getBool("mqtt:ssl:secure", secure);

    if (!secure) {
        logger.warn("MQTT SSL - operating in secure mode, but certificates will not be validated.");
        wifiClientSecure.setInsecure();
        return true;
    }

    char certFileName[128];

    if (!configuration.getString("mqtt:ssl:certFile", certFileName, sizeof(certFileName))) {
        logger.error("MQTT SSL - certificate file not configured, operating in insecure mode.");
        return false;
    }

    if (!LittleFS.exists(certFileName)) {
        logger.error("MQTT SSL - certificate file not found, operating in insecure mode.");
        return false;
    }

    File f = LittleFS.open(certFileName, "r");

    if (!f) {
        logger.error("MQTT SSL - failed to open certificate file.");
        return false;
    }

    BearSSL::X509List* serverTrustedCA = new BearSSL::X509List(f);
    f.close();
    
    if (!serverTrustedCA) {
        logger.error("MQTT SSL - failed to parse certificate file.");
        return false;
    }

    wifiClientSecure.setTrustAnchors(serverTrustedCA);

    logger.info("MQTT SSL - certificate loaded successfully.");

    return true;
}

char url[1024] = "";

bool setupMqttClient() {
  if (!configuration.getString("mqtt:url", url, sizeof(url))) {
      logger.error("MQTT - URL missing in configuration.");
      return false;
  }

  for (size_t i = 0; i < strlen(url); i++) {
    logger.info("url[%d] = %d", i, url[i]);
  }

  int port = 1883;
  if (!configuration.getInt("mqtt:port", port)) {
      logger.warn("MQTT - Port not found in configuration, using default 1883.");
  }
  
  logger.info("Connecting to %s:%d", url, port);

  client.setServer(url, port);
  client.setCallback(callback);
  return true;
}


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

  logger.info("----------");
  listFiles("./");

  // --------- Initialize
  logger.info("--------- Initializing");
  pinMode(HUMAN_PRESENCE_Pin, INPUT); 
  dht.begin();
  radar.HumanStatic_func(false);
  configuration.load();
  wifi.setup();

  // --------- Start Connections
  if (!setupCaCertificate()) {
    logger.error("Failed to initialize wifi secure client.");
    delay(5000);
    watchdog_reboot(0, 0, 0);
  }

  if (!setupClock()) {
    logger.error("Failed to connect with wifi.");
    delay(5000);
    watchdog_reboot(0, 0, 0);
  }

  if (!setupMqttClient()) {
    logger.error("Failed to setup mqtt");
    delay(5000);
    watchdog_reboot(0, 0, 0);
  }

  logger.info("--------- Initialized");
}

int i = 0;
int timeout = 100;
static int lastButtonState = LOW; 

bool loop_reconnect() {
  char err_buf[256] = "";

  char baseClientId[64] = "Client";
  if (!configuration.getString("mqtt:clientId", baseClientId, sizeof(baseClientId))){
    logger.warn("MQTT - baseclient ID missing in configuration, using default 'Client'.");
    return false;
  }

  char username[256] = "";
  if (!configuration.getString("mqtt:username", username, sizeof(username))){
    logger.error("MQTT - missing username.");
    return false;
  }

  char password[256] = "";
  if (!configuration.getString("mqtt:password", password, sizeof(password))){
    logger.error("MQTT - missing password.");
    return false;
  }

  const char* willTopic   = "device/room/sensor/set/1/status";
  const char* willMessage = "false";

  
  while (true) {
  logger.info("Credentails for mqtt connect: %s, %s", username, password);

    if (client.connected()) {
      break;
    }

    if (client.connect("Test", username, password, willTopic, 1, true, willMessage)) {
      logger.info("Mqtt Connected.");
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
    } else {
      logger.error("--- Mqtt not connected --- CPU Freq: %d, Heap: %d ", rp2040.f_cpu(), rp2040.getFreeHeap());
      wifiClientSecure.getLastSSLError(err_buf, sizeof(err_buf));
      logger.warn("SSL error: %s", err_buf);
      delay(2000);
    }
  }

  return true;
}

void loop() {
  i++;

  bool wifiStatus = wifi.loop();
  if (!wifiStatus) {
    logger.warn("Wifi connection failed...");
    delay(500);
    return;
  }

  

  //logger.info("--------- Monitor MQTT");

  if (!client.connected()) {
    bool mqttState = loop_reconnect();

    if (!mqttState) {
      logger.info("Failed to connect with mqtt server.");
      return;
    }

  }
  
  client.loop();

  //logger.info("--------- Monitor DHT");

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

  //logger.info("--------- Monitor Presence");

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