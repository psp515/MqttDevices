#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "JsonConfiguration.h"
#include "SerialLogger.h"

using namespace smartdevices::configuration;
using namespace smartdevices::logging;

const char* filename = "hivemq.pem";
String fileContent = R"EOF(-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

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

  //----------- Save File
  File f = LittleFS.open(filename, "w");
  if (f) {
    f.print(fileContent);
    f.close();
  }
  

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
