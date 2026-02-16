#include "MqttTransport.h"

namespace smartdevices::transport {

    MqttTransport::MqttTransport(Configuration& configuration, Logger& logger)
        : Transport(configuration, logger)
    {
        char url[1024];

        if (!_configuration.getString("mqtt:url", url, sizeof(url)))
            throw std::runtime_error("MQTT URL missing in configuration");

        int port = 1883;
        if (_configuration.getInt("mqtt:port", port))
            throw std::runtime_error("MQTT Port missing in configuration");

        bool sslEnabled = false;
        _configuration.getBool("mqtt:ssl:enabled", sslEnabled)

        if (sslEnabled) {
            _sslClient = new WiFiClientSecure();

            bool secure = false;
            _configuration.getBool("mqtt:ssl:secure", secure)

            char certFileName[1024];

            if (!_configuration.getString("mqtt:url", certFileName, sizeof(certFileName)))
                throw std::runtime_error("MQTT Ssl file name missing in configuration");

            setupCaCert(secure, certFileName);

            _mqttClient = new PubSubClient(*_sslClient);
        } else {
            _plainClient = new WiFiClient();
            _mqttClient = new PubSubClient(*_plainClient);
        }

        _mqttClient->setServer(url, port);
        _mqttClient->setCallback([this](char* topic, byte* payload, unsigned int length) 
        {
            _logger.debug("[" + topic + "]Received message: " + payload);
            mqttCallback(topic, payload, length);
        });
    }

    MqttTransport::~MqttTransport()
    {
        delete _mqttClient;
        delete _sslClient;
        delete _plainClient;
    }

    void setupCaCert() {
        bool secure = false;
        _configuration.getBool("mqtt:ssl:secure", secure)

        if (!secure) {
            _sslClient.setInsecure();
            return;
        }

        char certFileName[1024];

        if (!_configuration.getString("mqtt:ssl:certFile", certFileName, sizeof(certFileName)))
            throw std::runtime_error("MQTT Ssl file name missing in configuration");

        if (!LittleFS.exists(certFileName)) {
            throw std::runtime_error("MQTT SSL file not found.");
        }

        File f = LittleFS.open(certName, "r");

        if (!f) {
            throw std::runtime_error("MQTT SSL failed to open cert filename.");
        }

        BearSSL::X509List* caCert = new BearSSL::X509List(f);
        f.close();
        
        if (!caCert) {
            throw std::runtime_error("MQTT SSL failed to parse cert file.");
        }

        _sslClient.setTrustAnchors(caCert);

        logger.info("MQTT SSL - certificate loaded successfully.");
    }

    bool MqttTransport::start()
    {
        reconnect();
        return _mqttClient->connected();
    }


    // TODO: Verify Implementations

    void MqttTransport::reconnect()
    {
        char clientId[128];
        char baseTopic[64] = "";

        if (!_configuration.getString("mqtt:baseTopic", baseTopic, sizeof(baseTopic))) {
            strncpy(baseTopic, "device/default", sizeof(baseTopic) - 1);
            baseTopic[sizeof(baseTopic) - 1] = '\0';
        }

        char baseId[16] = "ESPClient";
        _configuration.getString("mqtt:clientId", baseId, sizeof(baseId));

        while (!_mqttClient->connected()) {
            char baseId[112] = "MqttClient"; 
            _configuration.getString("mqtt:clientId", baseId, sizeof(baseId));

            uint16_t randNum = random(0xffff);
            snprintf(clientId, sizeof(clientId), "%s-%04X", baseId, randNum);

            if (_mqttClient->connect(clientId)) {
                for (auto& obs : _observers) {
                    char fullTopic[128];
                    snprintf(fullTopic, sizeof(fullTopic), "%s/%s", baseTopic, obs.first); // prepend baseTopic
                    _mqttClient->subscribe(fullTopic);
                }
            } else {
                delay(500);
            }
        }
    }


    bool MqttTransport::send(TransportMessage message)
    {
        if (!_mqttClient->connected())
            reconnect();

        return _mqttClient->publish(message.getPath(), message.getPayload());
    }

    void MqttTransport::receive()
    {
        _mqttClient->loop();
    }

    void MqttTransport::observe(const char* id, MessageCallback callback)
    {
        Observer obs;
        obs.topic = id;
        obs.callback = callback;
        _observers.push_back(obs);

        if (_mqttClient->connected())
            _mqttClient->subscribe(id);
    }

    void MqttTransport::mqttCallback(char* topic, byte* payload, unsigned int length)
    {
        std::string strPayload((char*)payload, length);

        for (auto& obs : _observers) {
            if (strcmp(topic, obs.topic) == 0) {
                TransportMessage msg(topic, strPayload.c_str());
                obs.callback(msg);
            }
        }
    }

}
