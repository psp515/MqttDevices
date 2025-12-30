#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

const char CONFIGURATION_FILENAME[] = "/appsettings.json";

bool setupSettings(JsonDocument& doc) {
  Serial.println("Settings - initialization");

  if (!LittleFS.exists(CONFIGURATION_FILENAME)) {
    Serial.print("File not found: ");
    Serial.println(CONFIGURATION_FILENAME);
    return false;
  }

  File f = LittleFS.open(CONFIGURATION_FILENAME, "r");
  if (!f) {
    Serial.println("Failed to open file");
    return false;
  }

  DeserializationError error = deserializeJson(doc, f);
  f.close();

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return false;
  }

  return true;
}

bool setupCaCert(bool enabled, String certName, BearSSL::WiFiClientSecure& wifiClient) {
  Serial.println("SSL - initializing");

  if (!enabled) {
    wifiClient.setInsecure();
    Serial.println("SSL - insecure setup");
    return true;
  }

  if (!LittleFS.exists(certName)) {
      Serial.print("SSL - certificate file not found: ");
      Serial.println(certName);
      return false;
  }

  File f = LittleFS.open(certName, "r");

  if (!f) {
    Serial.print("SSL - failed to open certificate: ");
    Serial.println(certName);
    return false;
  }

  BearSSL::X509List*  caCert = new BearSSL::X509List(f);
  f.close();
  
  if (!caCert) {
      Serial.println("SSL - failed to parse certificate");
      return false;
  }

  wifiClient.setTrustAnchors(caCert);
  Serial.println("SSL - certificate loaded successfully");

  return true;
}

bool setupWifi(String ssid, String password) {
  Serial.println("Wifi - connecting to " + ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int i = 0;
  bool connected = false;
  while (true) {
    wl_status_t status = WiFi.status();

    if (WL_CONNECTED == status) {
      connected = true;
      break;
    } else if (WL_IDLE_STATUS == status) {
      break;
    }

    if (i == 100) {
      break;
    }

    i++;
    delay(100);
  }

  if (connected) {
    Serial.println("Wifi - connected.");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("Wifi - failed to connect. Reason");
  Serial.println(WiFi.status());
  return false;
}

bool setupClock() {
  Serial.println("NTP - Waiting for time sync");

  configTime(0, 0, "time.google.com", "time.nist.gov");
  time_t now = time(nullptr);

  int i = 0;
  while (now < 8 * 3600 * 2) {
    delay(100);
    now = time(nullptr);

    if (i == 50) {
      break;
    }

    i++;
  }

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("NTP - Current time utc: ");
  Serial.println(asctime(&timeinfo));
  return true;
}

class RemoteButtonMqttClient {
private: 
  WiFiClientSecure& _wifiClient;
  PubSubClient _mqttClient;
  const char* _url;
  uint16_t _port;
  const char* _clientId;
  const char* _username;
  const char* _password;
  const char* _baseTopic;

public:
  RemoteButtonMqttClient(
    WiFiClientSecure& wifiClient,
    const char* url,
    uint16_t port,
    const char* clientId,
    const char* username,
    const char* password,
    const char* baseTopic
  ) : _wifiClient(wifiClient),
      _mqttClient(wifiClient),
      _url(url),
      _port(port),
      _clientId(clientId),
      _username(username),
      _password(password),
      _baseTopic(baseTopic) 
  {}

  bool setup() {
    Serial.println("MQTT - Connecting");

    _mqttClient.setServer(_url, _port);

    // Maximal 15 seconds delay
    if (_mqttClient.connect(_clientId, _username, _password)) {
      Serial.println("MQTT - Connected");
      return true;
    }

    int state = _mqttClient.state();
    Serial.print("MQTT - Failed to connect, state=");
    Serial.println(state);
    return false;
  }

  bool sendMessage(const char* topic, String value) {
    String fullTopic = String(_baseTopic) + "/" + topic;
    bool result = _mqttClient.publish(fullTopic.c_str(), value.c_str());

    if (!result) {
      Serial.print("MQTT - Failed to publish to ");
      Serial.println(fullTopic);
    } else {
      Serial.print("MQTT - Successfully published message to ");
      Serial.println(fullTopic);
    }

    return result;
  }
};

class Led {
private:
  uint8_t _pin;
public:
  Led(uint8_t pin) : _pin(pin) 
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  void blink() {
    Serial.println("---- blink");
    digitalWrite(_pin, HIGH);
    delay(50);
    digitalWrite(_pin, LOW);
    delay(50);
  }

  void blinkError() {
    blink();
    blink();
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("");

  if (!LittleFS.begin()) {
    Serial.println("Device - LittleFS mount failed");
    return;
  }

  StaticJsonDocument<1024> settings;

  if (!setupSettings(settings)) {
    Serial.println("Failed to load settings");
    return;
  }

  Led builtin(14);

  String reason = ESP.getResetReason();

  if (reason == "Power On") {
    Serial.println("Device - just powered on, no action.");
    builtin.blink();
    return;
  } else if (reason != "Deep-Sleep Wake") {
    Serial.println("Device - unknown startup reason:");
    Serial.println(reason);
    builtin.blink();
    return;
  }

  unsigned long startTime = millis();
  Serial.println("Device - invoking work.");

  BearSSL::WiFiClientSecure wifiClient;

  if (!setupCaCert(settings["ssl"]["enabled"], settings["ssl"]["caCertFilename"], wifiClient)) {
    Serial.println("Device - failed to invoke work, certificates not configured");
    builtin.blinkError();
    return;
  }

  if (!setupWifi(settings["wifi"]["ssid"], settings["wifi"]["password"])) {
    Serial.println("Device - failed to invoke work, wifi not connected");
    builtin.blinkError();
    return;
  }

  Serial.print("Device - wifi connected, took ");
  Serial.println(millis() - startTime);

  if (!setupClock()) {
    Serial.println("Device - failed to invoke work, clock not updated.");
    builtin.blinkError();
    return;
  }

  RemoteButtonMqttClient mqttClient(wifiClient,
    settings["mqtt"]["url"],
    settings["mqtt"]["port"],
    settings["mqtt"]["clientId"],
    settings["mqtt"]["username"],
    settings["mqtt"]["password"],
    settings["mqtt"]["baseTopic"]);
  
  if (!mqttClient.setup()) {
    Serial.println("Device - failed to invoke work, mqtt not connected.");
    builtin.blinkError();
    return;
  }

  Serial.print("Device - mqtt connected, took ");
  Serial.println(millis() - startTime);

  time_t now = time(nullptr);
  struct tm* t = gmtime(&now);
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", t);

  unsigned long duration = millis() - startTime;

  String pressedEvent = "{ \"executionTime\": " + String(duration) +
                        ", \"timeUtc\": \"" + String(buf) +
                        "\", \"type\": \"pressed\" }";

  if (!mqttClient.sendMessage("event", pressedEvent)) {
    Serial.print("Device - failed to send event");
    builtin.blinkError();
  }

  Serial.print("Device - total work, took ");
  Serial.println(millis() - startTime);

  Serial.println("Device - invoke finished.");
  builtin.blink();
}

void loop() {
  Serial.println("Device - entering deep sleep.");
  ESP.deepSleep(0);
}