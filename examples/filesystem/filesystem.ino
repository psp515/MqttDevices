#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "JsonConfiguration.h"
#include "SerialLogger.h"

using namespace smartdevices::configuration;
using namespace smartdevices::logging;

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

SerialLogger logger(Serial, LogLevel::DEBUG);
JsonConfiguration config(logger, filename);

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
  logger.info("----------- List files ----------- ");
  listFiles("./");

  // ----------- Print File
  logger.info("----------- Print file content ----------- ");
  printFileContent("appsettings.json");

  // ----------- Json
  logger.info("----------- Json lib example ----------- ");
  File f = LittleFS.open(filename, "r");
  String data = f.readString();

  JsonDocument doc;
  auto err = deserializeJson(doc, data.c_str());

  if (err) {
    Serial.println(err.c_str());  
  }

  logger.info("Found element in json: %s", doc["wifi"]["password"].as<const char*>());

  // Json Configuration Example
  logger.info("----------- Config example ----------- ");
  config.load();

  char password[64] = "\0";
  bool success = config.getString("wifi:password", password, sizeof(password));

  logger.info("Loaded configuration value: %s", password);

  int port = 0;
  success = config.getInt("mqtt:port", port);

  char buffer[64];
  snprintf(buffer, sizeof(buffer), "Loaded configuration value port: %d", port);
  logger.warn(buffer);

  logger.info("Sleeping ...");  
  delay(1000);
}
