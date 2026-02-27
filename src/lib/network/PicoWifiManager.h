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
            : WifiManager(config, logger),
            _interval(5000),
            _maxInterval(40000),
            _lastAttemptTime(0),
            _connected(false)
        {
            memset(_ssid, 0, sizeof(_ssid));
            memset(_password, 0, sizeof(_password));
        }

        bool setup() override
        {
            if (!_config.getString("wifi:ssid", _ssid, sizeof(_ssid))) {
                _logger.error("[WiFi] SSID missing in configuration.");
                return false;
            }

            if (!_config.getString("wifi:password", _password, sizeof(_password))) {
                _logger.error("[WiFi] Password missing in configuration.");
                return false;
            }

            WiFi.mode(WIFI_STA);

            return true;
        }

        bool loop() override
        {
            unsigned long now = millis();

            if (WiFi.status() == WL_CONNECTED) {

                if (!_connected) {
                    _connected = true;
                    _interval = _defaultInterval;
                    _logger.info("[WiFi] Connected. IP: %s",
                                WiFi.localIP().toString().c_str());
                }

                return true;
            }

            if (_connected) {
                _connected = false;
                _logger.warn("[WiFi] Connection lost.");
                _lastAttemptTime = now;
            }

            if (now - _lastAttemptTime >= _interval) {
                _logger.warn("[WiFi] Reconnect attempt. Interval: %lu ms", _interval);

                startAttempt();

                _interval *= 2;
                if (_interval > _maxInterval)
                    _interval = _maxInterval;
            }

            return false;
        }

        bool isConnected() const override
        {
            return WiFi.status() == WL_CONNECTED;
        }

        bool reconnect() override
        {
            _interval = _defaultInterval;
            _lastAttemptTime = 0;
            setup();
            startAttempt();
            return true;
        }

    private:
        char _ssid[256];
        char _password[256];

        unsigned long _defaultInterval = 5000;
        unsigned long _interval;
        const unsigned long _maxInterval;
        unsigned long _lastAttemptTime;

        bool _connected;

        void startAttempt()
        {
            WiFi.disconnect();
            WiFi.begin(_ssid, _password);
            _lastAttemptTime = millis();
        }
    };

}

#endif