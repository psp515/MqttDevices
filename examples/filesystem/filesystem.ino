#include <Arduino.h>
#include <LittleFS.h>
#include <user_interface.h>

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
  Serial.begin(115200);
    
  Serial.println("");
  Serial.println("Uploading file");

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  listFiles("./");

  printFileContent("appsettings.json");

  // File f = LittleFS.open(filename, "w");
  // if (f) {
  //   f.print(fileContent);
  //   f.close();
  // }
}

void loop() {
  Serial.println("Deep sleeping");
  ESP.deepSleep(6);
}
