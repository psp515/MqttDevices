#ifndef PICO_WIFI_MANAGER_H
#define PICO_WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

#include "Logger.h"
#include "Configuration.h"
#include "WifiManager.h"

namespace smartdevices::network {

    using namespace smartdevices::configuration;
    using namespace smartdevices::logging;


    class PicoWifiManager final : public WifiManager
    {
    public:
        PicoWifiManager(Configuration& config, Logger& logger)
            :  WifiManager(config, logger),
              _lastReconnectAttempt(0),
              _reconnectInterval(5000),
              _maxReconnectInterval(60000),
              _connected(false),
              _firstFailureTime(0)
        {
            memset(_ssid, 0, sizeof(_ssid));
            memset(_password, 0, sizeof(_password));
        }

        bool setup() override {

            if (!_config.getString("wifi:ssid", _ssid, sizeof(_ssid))) {
                _logger.error("WiFi SSID missing in configuration.");
                return false;
            }

            if (!_config.getString("wifi:password", _password, sizeof(_password))) {
                _logger.error("WiFi password missing in configuration.");
                return false;
            }

            WiFi.mode(WIFI_STA);
            WiFi.begin(_ssid, _password);

            return waitForConnection(15000);
        }

        bool loop() override {

            if (WiFi.status() == WL_CONNECTED) {

                if (!_connected) {
                    _connected = true;
                    _logger.info("WiFi connected. IP: %s", WiFi.localIP().toString().c_str());
                }

                _reconnectInterval = 5000;
                _firstFailureTime = 0;

                return true;
            }

            _connected = false;

            unsigned long now = millis();

            if (_firstFailureTime == 0)
                _firstFailureTime = now;

            if (now - _firstFailureTime > 300000) {
                _logger.critical("WiFi offline too long. Triggering watchdog reset.");
                triggerWatchdogReset();
            }

            if (now - _lastReconnectAttempt < _reconnectInterval)
                return false;

            _lastReconnectAttempt = now;

            _logger.warn("WiFi reconnect attempt. Interval: %lu ms",
                         _reconnectInterval);

            WiFi.disconnect();
            WiFi.begin(_ssid, _password);

            _reconnectInterval *= 2;
            if (_reconnectInterval > _maxReconnectInterval)
                _reconnectInterval = _maxReconnectInterval;

            return false;
        }

        bool isConnected() const override {
            return WiFi.status() == WL_CONNECTED;
        }

        bool reconnect() override {
            return setup();
        }

    private:
        char _ssid[256];
        char _password[256];

        unsigned long _lastReconnectAttempt;
        unsigned long _reconnectInterval;
        unsigned long _maxReconnectInterval;

        bool _connected;
        unsigned long _firstFailureTime;
        
        bool waitForConnection(unsigned long timeoutMs) {

            unsigned long start = millis();

            while (WiFi.status() != WL_CONNECTED) {
                delay(250);

                if (millis() - start > timeoutMs) {
                    _logger.error("Initial WiFi connection timeout.");
                    return false;
                }
            }

            _connected = true;
            _logger.info("WiFi connected. IP: %s",
                         WiFi.localIP().toString().c_str());

            return true;
        }

        void triggerWatchdogReset() {
            rp2040.wdt_begin(1000);
        }
    };

}

#endif