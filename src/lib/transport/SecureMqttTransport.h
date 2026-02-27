#ifndef SECURE_MQTT_TRANSPORT_H
#define SECURE_MQTT_TRANSPORT_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <cstring>

#include "Logger.h"
#include "Configuration.h"
#include "Transport.h"

namespace smartdevices::transport {

using namespace smartdevices::logging;
using namespace smartdevices::configuration;

class MqttTransportMessage : public TransportMessage {
public:
    MqttTransportMessage() = default;

    MqttTransportMessage(const char* topic,
                         const uint8_t* payload,
                         unsigned int length,
                         bool retained = false)
        : retained(retained)
    {
        if (topic) {
            path = std::string(topic);
        }

        if (payload && length > 0) {
            this->payload = std::string(
                reinterpret_cast<const char*>(payload),
                length
            );
        }
    }

    bool isRetained() const {
        return retained;
    }

private:
    bool retained;
};

class SecureMqttTransport final : public Transport {
public:
    explicit SecureMqttTransport(Configuration& configuration, Logger& logger)
        : Transport(configuration, logger),
          _serverTrustedCA(nullptr),
          _client(_wifiClientSecure),
          _connected(false),
          _lastReconnectAttempt(0),
          _reconnectInterval(2000),
          _defaultReconnectInterval(2000),
          _maxReconnectInterval(60000)
    {
        memset(_url, 0, sizeof(_url));
        memset(_username, 0, sizeof(_username));
        memset(_password, 0, sizeof(_password));
        memset(_clientId, 0, sizeof(_clientId));
        memset(_baseTopic, 0, sizeof(_baseTopic));
        memset(_stateTopic, 0, sizeof(_stateTopic));

        _instance = this;
    }

    ~SecureMqttTransport() override {
        delete _serverTrustedCA;
    }

    bool setup() override {

        if (!_configuration.getString("mqtt:url", _url, sizeof(_url)))
            return false;

        if (!_configuration.getString("mqtt:username", _username, sizeof(_username)))
            return false;

        if (!_configuration.getString("mqtt:password", _password, sizeof(_password)))
            return false;

        if (!_configuration.getString("mqtt:clientId", _clientId, sizeof(_clientId)))
            return false;

        if (!_configuration.getString("mqtt:baseTopic", _baseTopic, sizeof(_baseTopic)))
            return false;

        snprintf(_stateTopic, sizeof(_stateTopic), "%s/status", _baseTopic);

        if (!_configuration.getInt("mqtt:port", _port))
            return false;

        setupCaCertificate();
        _client.setServer(_url, _port);
        _client.setCallback(mqttCallbackStatic);

        return true;
    }

    bool loop() override {

        unsigned long now = millis();

        if (_client.connected()) {

            if (!_connected) {
                _connected = true;
            }

            _client.loop();
            _reconnectInterval = _defaultReconnectInterval;
            return true;
        }

        _connected = false;

        if (now - _lastReconnectAttempt >= _reconnectInterval) {

            _lastReconnectAttempt = now;

            if (_client.connect(
                    _clientId,
                    _username,
                    _password,
                    _stateTopic,
                    1,
                    true,
                    _stateDisconnectedPayload))
            {
                _client.publish(_stateTopic, _stateConnectedPayload, true);
                _connected = true;
                _reconnectInterval = _defaultReconnectInterval;

                for (auto& cb : _callbacks) {
                    _client.subscribe(cb.path.c_str());
                }

                return true;
            }

            _reconnectInterval *= 2;
            if (_reconnectInterval > _maxReconnectInterval)
                _reconnectInterval = _maxReconnectInterval;
        }

        return false;
    }

    bool reconnect() override {
        return setup();
    }

    bool send(const TransportMessage& message) override {

        if (!_client.connected())
            return false;

        std::string fullPath;
        if (!message.getPath().empty() && message.getPath()[0] != '/') {
            fullPath = std::string(_baseTopic) + "/" + message.getPath();
        } else {
            fullPath = message.getPath();
        }

        return _client.publish(
            fullPath.c_str(),
            message.getPayload().c_str()
        );
    }

    bool observe(const std::string& path,
                 MessageCallback callback) override
    {
        for (auto& cb : _callbacks) {
            if (cb.path == path)
                return true;
        }

        _callbacks.push_back({path, callback});

        if (_client.connected()) {
            return _client.subscribe(path.c_str());
        }

        return true;
    }

private:

    static void mqttCallbackStatic(char* topic,
                                   uint8_t* payload,
                                   unsigned int length)
    {
        if (_instance) {
            _instance->mqttCallback(topic, payload, length);
        }
    }

    void mqttCallback(char* topic,
                      uint8_t* payload,
                      unsigned int length)
    {
        MqttTransportMessage msg(topic, payload, length);

        for (auto& cb : _callbacks) {
            if (cb.path == msg.getPath()) {
                cb.callback(msg);
            }
        }
    }

    bool setupCaCertificate() {

        bool secureEnabled = true;

        if (!_configuration.getBool("mqtt:ssl:enabled", secureEnabled) ||
            !secureEnabled)
        {
            _logger.info("[MQTT] SSL/TLS is disabled in configuration. Setting insecure mode.");
            _wifiClientSecure.setInsecure();
            return true;
        }

        delete _serverTrustedCA;
        _serverTrustedCA = nullptr;

        char certFile[128];

        if (!_configuration.getString("mqtt:ssl:certFile", certFile, sizeof(certFile))) {
            
            _logger.error("[MQTT] SSL/TLS is enabled but 'mqtt:ssl:certFile' is not set in configuration.");
            return false;
        }

        if (!LittleFS.exists(certFile)) {
            _logger.error("[MQTT] SSL/TLS certificate file '%s' not found in LittleFS.", certFile);
            return false;
        }

        File f = LittleFS.open(certFile, "r");
        if (!f) {
            _logger.error("[MQTT] Failed to open SSL/TLS certificate file '%s'.", certFile);
            return false;
        }

        _serverTrustedCA = new BearSSL::X509List(f);
        f.close();

        if (!_serverTrustedCA) {
            _logger.error("[MQTT] Failed to load SSL/TLS certificate from file '%s'.", certFile);
            return false;
        }

        _wifiClientSecure.setTrustAnchors(_serverTrustedCA);
        _logger.info("[MQTT] SSL/TLS certificate loaded successfully from '%s'.", certFile);
        return true;
    }

private:
    static SecureMqttTransport* _instance;

    BearSSL::X509List* _serverTrustedCA;
    BearSSL::WiFiClientSecure _wifiClientSecure;
    PubSubClient _client;

    char _stateDisconnectedPayload[16] = "false";
    char _stateConnectedPayload[16] = "true";
    char _stateTopic[256];

    char _url[1024];
    char _username[1024];
    char _password[1024];
    char _clientId[256];
    char _baseTopic[256];

    int _port{8883};

    bool _connected;

    unsigned long _lastReconnectAttempt;
    unsigned long _reconnectInterval;
    unsigned long _defaultReconnectInterval;
    unsigned long _maxReconnectInterval;
};

} // namespace smartdevices::transport

namespace smartdevices::transport {
    SecureMqttTransport* SecureMqttTransport::_instance = nullptr;
}

#endif