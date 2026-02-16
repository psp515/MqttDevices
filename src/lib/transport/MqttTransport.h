#ifndef MQTT_TRANSPORT_H
#define MQTT_TRANSPORT_H

#include "Transport.h"
#include "Logger.h"

using namespace smartdevices::logging;

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