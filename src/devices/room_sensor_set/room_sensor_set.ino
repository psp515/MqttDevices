#include <Arduino.h>
#include <humanstaticLite.h>
#include <SoftwareSerial.h>
#include <String.h>
#include <DHT.h>

#include "Logger.h"
#include "SerialLogger.h"

#include "MqttTransport.h"
#include "Configuration.h"
#include "JsonConfiguration.h"

#define DHTPIN 14  
#define DHTTYPE DHT22  

#define RX_Pin 5
#define TX_Pin 4
#define PRESENCE_Pin 15

using namespace smartdevices::logging;

SoftwareSerial mySerial = SoftwareSerial(RX_Pin, TX_Pin);
HumanStaticLite radar = HumanStaticLite(&mySerial);

SerialLogger serialLogger(Serial);
Logger& logger = serialLogger;
DHT dht(DHTPIN, DHTTYPE);

JsonConfiguration jsonConfiguration("appsettings.json");

Logger& logger = serialLogger;
Configuration& configuration = jsonConfiguration;

void setup() {
  Serial.begin(115200);
  mySerial.begin(115200);
  dht.begin();

  char* password = configuration.getString("wifi:password");

  pinMode(PRESENCE_Pin, INPUT); 

  logger.info("Device initialized!");
}


int i = 0;


static int lastButtonState = LOW; 

void loop() {
  radar.HumanStatic_func(false);
  i++;

  if (i%50 == 0) {
    i = 0;

    float h = dht.readHumidity();
    float t = dht.readTemperature();     // Celsius
    float f = dht.readTemperature(true); // Fahrenheit

    if (isnan(h) || isnan(t) || isnan(f)) {
      logger.error("Failed to read from DHT sensor!");
      return;
    }

    Serial.println(String("Temp: ") + dht.readTemperature() + "°C  Hum: " + dht.readHumidity() + "%");
  }

  int buttonState = digitalRead(PRESENCE_Pin);

  if (buttonState != lastButtonState) {
      lastButtonState = buttonState;

      if (buttonState == HIGH) {
          logger.info("Human present.");
      } else {
          logger.info("No human present.");
      }
  }

  if(radar.radarStatus != 0x00){
    
    //radar.showData();

    switch(radar.radarStatus){
      case SOMEONE:
        logger.info("Someone is here.");
        logger.info("---------------------------------");
        break;
      case NOONE:
        logger.info("Nobody here.");
        logger.info("---------------------------------");
        break;
      case NOTHING:
        logger.info("No human activity messages.");
        logger.info("---------------------------------");
        break;
      case SOMEONE_STOP:
        logger.info("Someone stop");
        logger.info("---------------------------------");
        break;
      case SOMEONE_MOVE:
        logger.info("Someone moving");
        logger.info("---------------------------------");
        break;
      case HUMANPARA:
        logger.info("The parameters of human body signs are: ");
        logger.info(String(radar.bodysign_val, DEC).c_str());
        logger.info("---------------------------------");
        break;
      case SOMEONE_CLOSE:
        logger.info("Someone is closing");
        logger.info("---------------------------------");
        break;
      case SOMEONE_AWAY:
        logger.info("Someone is staying away");
        logger.info("---------------------------------");
        break;
      case DETAILMESSAGE:
        logger.info("Spatial static values: ");
        logger.info(String(radar.static_val, DEC).c_str());
        logger.info("Distance to stationary object: ");
        logger.info(String(radar.dynamic_val, DEC).c_str());
        logger.info("Spatial dynamic values: ");
        logger.info(String(radar.dis_static, DEC).c_str());
        logger.info("Distance from the movement object: ");
        logger.info(String(radar.dis_move, DEC).c_str());
        logger.info("Speed of moving object: ");
        logger.info(String(radar.speed, DEC).c_str());
        logger.info("---------------------------------");
        break;
    }
  }

  delay(200);
}