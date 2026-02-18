#include <string>
#include <LittleFS.h>

#include "MqttTransport.h"

namespace smartdevices::transport {

    // ---------------------- Static 

    MqttTransport* MqttTransport::_instance = nullptr;

    void MqttTransport::mqttCallbackStatic(char* topic, byte* payload, unsigned int length)
    {
        if (_instance) {
            _instance->mqttCallback(topic, payload, length);
        }
    }

    // ---------------------- Implementation

    MqttTransport::MqttTransport(Configuration& configuration, Logger& logger)
        : Transport(configuration, logger)
    {
        _instance = this;

        bool sslEnabled = false;
        configuration.getBool("mqtt:ssl:enabled", sslEnabled);

        if (sslEnabled) {
            _sslClient = new WiFiClientSecure();
            _mqttClient = PubSubClient(*_sslClient);
        } else {
            _plainClient = new WiFiClient();
            _mqttClient = PubSubClient(*_plainClient);
        }
    }

    MqttTransport::~MqttTransport()
    {
        delete _sslClient;
        delete _plainClient;
        _instance = nullptr;
    }

    bool MqttTransport::start()
    {
        char url[1024] = "";

        if (!_configuration.getString("mqtt:url", url, sizeof(url))) {
            _logger.error("MQTT - URL missing in configuration.");
            return false;
        }

        int port = 1883;
        if (_configuration.getInt("mqtt:port", port)) {
            _logger.warn("MQTT - Port not found in configuration, using default 1883.");
        }
        
        initializeCaCertificate();

        _mqttClient.setServer(url, port);

        _mqttClient.setCallback(MqttTransport::mqttCallbackStatic);

        reconnect();

        return _mqttClient.connected();
    }

    bool MqttTransport::reconnect()
    {
        if (_mqttClient.connected()) {
            _logger.debug("MQTT - Already connected, no need to reconnect.");
            return true;
        }

        long now = millis();

        if (now - lastReconnectAttempt < 5000 && lastReconnectAttempt != 0) {
            
            _logger.debug("MQTT - Reconnect attempted too recently, waiting before next attempt.");
            return false;
        }

        lastReconnectAttempt = now;

        char baseTopic[128] = "";

        if (!_configuration.getString("mqtt:baseTopic", baseTopic, sizeof(baseTopic))) {
            _logger.error("MQTT - base topic missing in configuration.");
            return false;
        }

        char baseClientId[64] = "Client";
        if (!_configuration.getString("mqtt:clientId", baseClientId, sizeof(baseClientId))){
            _logger.warn("MQTT - baseclient ID missing in configuration, using default 'Client'.");
        }

        int i = 0;

        while (!_mqttClient.connected() && i < 10) {
            char clientId[128] = "Client"; 

            uint16_t randNum = random(0xffff);
            snprintf(clientId, sizeof(clientId), "%s-%04X", baseClientId, randNum);

            if (_mqttClient.connect(clientId)) {

                for (auto& callback : _callbacks) {
                    char fullTopic[128];
                    snprintf(fullTopic, sizeof(fullTopic), "%s/%s", baseTopic, callback.path);
                    _mqttClient.subscribe(fullTopic);
                }

            } else {
                _logger.warn("MQTT - Failed to connect, retrying in 2 seconds...");
                delay(2000);
            }

            i++;
        }

        return _mqttClient.connected();
    }

    bool MqttTransport::send(TransportMessage message)
    {
        if (!_mqttClient.connected()) {
            
            _logger.warn("MQTT - Not connected when sending message, attempting to reconnect...");

            if (!reconnect()) {
                _logger.error("MQTT - Failed to reconnect, message not sent.");
                return false;
            }
        }

        return _mqttClient.publish(message.getPath(), message.getPayload());
    }

    bool MqttTransport::loop()
    {
        if (!_mqttClient.connected()) {
            
            _logger.warn("MQTT - Not connected when standard looping, attempting to reconnect...");

            if (!reconnect()) {
                _logger.error("MQTT - Failed to reconnect, message not sent.");
                return false;
            }
        }
    
        _mqttClient.loop();

        return true;
    }

    bool MqttTransport::observe(const char* id, MessageCallback callback)
    {
        TransportCallback transportCallback;
        transportCallback.path = id;
        transportCallback.callback = callback;
        _callbacks.push_back(transportCallback);

        if (_mqttClient.connected()) {
            if (!initializeCallback(transportCallback)) {
                _logger.error("MQTT - Failed to subscribe to topic for callback.");
                return false;
            }
        }

        return true;
    }

    bool MqttTransport::initializeCallback(const TransportCallback& callback) {

        char baseTopic[256] = "";
        _configuration.getString("mqtt:baseTopic", baseTopic, sizeof(baseTopic));

        char fullTopic[512];
        snprintf(fullTopic, sizeof(fullTopic), "%s/%s", baseTopic, callback.path);

       return _mqttClient.subscribe(fullTopic);
    }

    bool MqttTransport::initializeCaCertificate() {
        _logger.info("MQTT SSL - initializing CA certificate.");

        bool secureClientEnabled = false;
        _configuration.getBool("mqtt:ssl:enabled", secureClientEnabled);

        if (!secureClientEnabled) {
            _logger.warn("MQTT SSL - operating in insecure mode, certificates will not be validated.");
            _sslClient->setInsecure();
            return true;
        }

        bool secure = false;
        _configuration.getBool("mqtt:ssl:secure", secure);

        if (!secure) {
            _logger.warn("MQTT SSL - operating in secure mode, but certificates will not be validated.");
            _sslClient->setInsecure();
            return true;
        }

        char certFileName[128];

        if (!_configuration.getString("mqtt:ssl:certFile", certFileName, sizeof(certFileName))) {
            _logger.error("MQTT SSL - certificate file not configured, operating in insecure mode.");
            return false;
        }

        if (!LittleFS.exists(certFileName)) {
            _logger.error("MQTT SSL - certificate file not found, operating in insecure mode.");
            return false;
        }

        File f = LittleFS.open(certFileName, "r");

        if (!f) {
            _logger.error("MQTT SSL - failed to open certificate file.");
            return false;
        }

        BearSSL::X509List* caCert = new BearSSL::X509List(f);
        f.close();
        
        if (!caCert) {
            _logger.error("MQTT SSL - failed to parse certificate file.");
            return false;
        }

        _sslClient->setTrustAnchors(caCert);

        _logger.info("MQTT SSL - certificate loaded successfully.");

        return true;
    }

    void MqttTransport::mqttCallback(char* topic, byte* payload, unsigned int length)
    {
        std::string strPayload((char*)payload, length);

        _logger.debug((std::string("[") + topic + "] Received message:" + strPayload).c_str());

        for (auto& callback : _callbacks) {
            if (strcmp(topic, callback.path) == 0) {
                TransportMessage msg(topic, strPayload.c_str());
                callback.callback(msg);
            }
        }
    }

}
