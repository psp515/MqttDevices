#include <Arduino.h>
#include <LittleFS.h>
#include <user_interface.h>

const char* filename = "/lastReset.txt";
const int buttonPin = 4;
const int ledPin = 2;

void setup() {
  Serial.begin(75880);

  String reson = ESP.getResetReason();

  if (reson == "Power On") {

  if (reson == "Deep-Sleep Wake") {
    
  }

  Serial.println("");
  Serial.print("Reset Reason: ");
  Serial.println(ESP.getResetReason());

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  String dir_path = "./";
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

  // Read last reset type from file
  int lastResetType = -1;
  if (LittleFS.exists(filename)) {
    File f = LittleFS.open(filename, "r");
    if (f) {
      lastResetType = f.parseInt();
      f.close();
    }
  }
  Serial.print("Last reset type was: ");
  Serial.println(lastResetType);

  // Detect current boot type
  int currentResetType = (digitalRead(buttonPin) == LOW) ? 1 : 0;

  if (currentResetType == 1) {
    Serial.println("Wake by button press detected");
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
  } else {
    Serial.println("Normal boot detected");
    for (int i = 0; i < 2; i++) {
      digitalWrite(ledPin, HIGH);
      delay(200);
      digitalWrite(ledPin, LOW);
    }
  }

  // Store current reset type to file for next boot
  File f = LittleFS.open(filename, "w");
  if (f) {
    f.print(currentResetType);
    f.close();
  }


}

void loop() {
  Serial.println("Deep sleeping");
  ESP.deepSleep(6);
}
