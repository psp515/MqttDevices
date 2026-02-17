#ifndef MQTT_TRANSPORT_H
#define MQTT_TRANSPORT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>


#include "Logger.h"
#include "Configuration.h"
#include "Transport.h"

using namespace smartdevices::logging;
using namespace smartdevices::configuration;

namespace smartdevices::transport {

    class MqttTransport final : public Transport {
    public:
        explicit MqttTransport(Configuration& configuration, Logger& logger);
        virtual ~MqttTransport();

    private:
        void mqttCallback(char* topic, byte* payload, unsigned int length);
        void setupCaCert();

        WiFiClientSecure* _sslClient = nullptr;
        WiFiClient* _plainClient = nullptr;
        PubSubClient* _mqttClient = nullptr;
    };
}

#endif