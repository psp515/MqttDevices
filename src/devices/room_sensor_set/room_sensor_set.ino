#include "Arduino.h"
#include <humanstaticLite.h>
#include <SoftwareSerial.h>


#define RX_Pin 5
#define TX_Pin 4

SoftwareSerial mySerial = SoftwareSerial(RX_Pin, TX_Pin);

HumanStaticLite radar = HumanStaticLite(&mySerial);

void setup() {
  mySerial.begin(115200);
  while(!mySerial);
  Serial.println("Readly");
}

void loop() {
  radar.HumanStatic_func(true);

  if(radar.radarStatus != 0x00){
    
    radar.showData();  
    Serial.println(radar.radarStatus);

    switch(radar.radarStatus){
      case SOMEONE:
        Serial.println("Someone is here.");
        Serial.println("---------------------------------");
        break;
      case NOONE:
        Serial.println("Nobody here.");
        Serial.println("---------------------------------");
        break;
      case NOTHING:
        Serial.println("No human activity messages.");
        Serial.println("---------------------------------");
        break;
      case SOMEONE_STOP:
        Serial.println("Someone stop");
        Serial.println("---------------------------------");
        break;
      case SOMEONE_MOVE:
        Serial.println("Someone moving");
        Serial.println("---------------------------------");
        break;
      case HUMANPARA:
        Serial.print("The parameters of human body signs are: ");
        Serial.println(radar.bodysign_val, DEC);
        Serial.println("---------------------------------");
        break;
      case SOMEONE_CLOSE:
        Serial.println("Someone is closing");
        Serial.println("---------------------------------");
        break;
      case SOMEONE_AWAY:
        Serial.println("Someone is staying away");
        Serial.println("---------------------------------");
        break;
      case DETAILMESSAGE:
        Serial.print("Spatial static values: ");
        Serial.println(radar.static_val, DEC);
        Serial.print("Distance to stationary object: ");
        Serial.println(radar.dynamic_val, DEC);
        Serial.print("Spatial dynamic values: ");
        Serial.println(radar.dis_static, DEC);
        Serial.print("Distance from the movement object: ");
        Serial.println(radar.dis_move, DEC);
        Serial.print("Speed of moving object: ");
        Serial.println(radar.speed, DEC);
        Serial.println("---------------------------------");
        break;
    }
  }

  delay(200);
}