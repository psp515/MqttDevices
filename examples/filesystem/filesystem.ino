#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "JsonConfiguration.h"

using namespace smartdevices::configuration;

const char* filename = "/appsettings.json";
String fileContent = R"EOF()EOF";

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

void removeAllFiles() {
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        LittleFS.remove(dir.fileName());
    }
}

void removeFile(String fileToRemove) {
  if (LittleFS.exists(fileToRemove)) {
      LittleFS.remove(fileToRemove);
  }
}

void printFileContent(String file) {
  if (LittleFS.exists(file)) {
      File f = LittleFS.open(file, "r");
      String data = f.readString();
      Serial.println(data);
  }
}

void setup() {
  delay(15000);
  Serial.begin(115200);
    
  Serial.println("");
  Serial.println("Uploading file");

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  // ----------- Save File
  // File f = LittleFS.open(filename, "w");
  // if (f) {
  //   f.print(fileContent);
  //   f.close();
  // }
  

}

void loop() {
  Serial.println("Working ...");   
  
  // ----------- List File
  listFiles("./");

  // ----------- Print File
  printFileContent("appsettings.json");

  // ----------- Json
  File f = LittleFS.open(filename, "r");
  String data = f.readString();

  JsonDocument doc;
  auto err = deserializeJson(doc, data.c_str());

  if (err) {
    Serial.println(err.c_str());  
  }

  Serial.println("Deserialization success.");  

  Serial.print("Found element in json: ");
  Serial.println(doc["wifi"]["password"].as<const char*>());

  // Json Configuration Example

  SerialLogger logger(Serial);
  JsonConfiguration config(logger, filename);

  char password[64]= config.getConfig()["wifi"]["password"];

  Serial.print("Loaded value from custom config:" );
  Serial.println(password);

  Serial.println("Sleeping ...");  
  delay(5000);
}
