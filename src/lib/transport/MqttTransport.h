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
        bool start() override;
        bool reconnect() override;
        bool send(TransportMessage message) override;
        bool loop() override;
        bool observe(const char* path, MessageCallback callback) override;
        PubSubClient getClient() {
            return _mqttClient;
        }
        
    private:
        static void mqttCallbackStatic(char* topic, byte* payload, unsigned int length);
        void mqttCallback(char* topic, byte* payload, unsigned int length);
        bool initializeCallback(const TransportCallback& callback);
        bool initializeCaCertificate();

        static MqttTransport* _instance;
        long lastReconnectAttempt = 0;
        WiFiClientSecure* _sslClient = nullptr;
        WiFiClient* _plainClient = nullptr;
        PubSubClient _mqttClient;
    };
}

#endif