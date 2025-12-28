#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <PubSubClient.h>

const String configuration_file_name = "appsettings.json";

static const char cert[] PROGMEM = R"EOF(

)EOF";

void blinkOnPin(int pin)
{
  pinMode(pin, OUTPUT);

  digitalWrite(pin, HIGH);
  delay(500);
  digitalWrite(pin, LOW);    
  delay(500);
}

bool setupWifi(String ssid, String password) {
  Serial.println("Wifi - connecting to " + ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  bool connected = false;
  while (true) {
    wl_status_t status = WiFi.status();

    if (WL_CONNECTED == status) {
      connected = true;
      break;
    } else if (WL_IDLE_STATUS == status) {
      break;
    }

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

  while (now < 8 * 3600 * 2) {
    delay(100);
    now = time(nullptr);
  }

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("NTP - Current time utc: ");
  Serial.println(asctime(&timeinfo));
  return true;
}


class MqttSimple {
  private: 
    WiFiClientSecure& _wifiClient;
    PubSubClient _mqttClient;
    const char* url;
    uint16_t _port;
    const char* _clientId;
    const char* _username
    const char* _password;
    const char* _baseTopis;
}

bool setupMqtt(
  PubSubClient& mqttClient, 
  const char* url, 
  uint16_t port, 
  const char* clientId,
  const char* username, 
  const char* password) {

  Serial.println("MQTT - Connecting");

  mqttClient.setServer(url, port);

  // Attempt connection once
  if (mqttClient.connect(clientId, username, password)) {
    Serial.println("MQTT - Connected");
    return true;
  }

  // Connection failed immediately, report error
  int state = mqttClient.state();
  Serial.print("MQTT - Failed to connect, state=");
  Serial.println(state);

  return false;
}

void setup() {
  Serial.begin(115200);
  Serial.println("");

  String reason = ESP.getResetReason();

  // int redPin = 10;
  // int bluePin = 4;
  // int greenPin = 5;

  // blinkOnPin(redPin);
  // blinkOnPin(bluePin);
  // blinkOnPin(greenPin);

  if (reason == "Power On") {
    Serial.println("Device - just powered on, no action.");
  } else if (reason != "Deep-Sleep Wake") {
    Serial.println("Device - unknown startup reason:");
    Serial.println(reason);
  } else {
    unsigned long startTime = millis();
    Serial.println("Device - invoking work.");

    BearSSL::WiFiClientSecure wifiClient;
    BearSSL::X509List *serverTrustedCA = new BearSSL::X509List(cert);
    wifiClient.setTrustAnchors(serverTrustedCA);

    bool wifiWorking = setupWifi();

    if (!wifiWorking) {
      Serial.println("Device - failed to invoke work, wifi not connected");
      return;
    }

    Serial.print("Device - wifi connected, took ");
    Serial.println(millis() - startTime);

    bool clockWorking = setupClock();

    if (!clockWorking) {
      Serial.println("Device - failed to invoke work, clock not updated.");
      return;
    }

    Serial.print("Device - clock working, took ");
    Serial.println(millis() - startTime);

    PubSubClient mqttClient(wifiClient);
    bool mqttWorking = setupMqtt(

    );

    if (!mqttWorking) {
      Serial.println("Device - failed to invoke work, mqtt not connected.");
      return;
    }

    Serial.print("Device - mqtt connected, took ");
    Serial.println(millis() - startTime);

    

    Serial.println("Device - invoke finished.");
  }
}

void loop() {
  Serial.println("Device - entering deep sleep.");
  ESP.deepSleep(0);
}