#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>          

#include "Configuration.h"
#include "Logger.h"

namespace smartdevices::network {

    using namespace smartdevices::configuration;
    using namespace smartdevices::logging;

    class WifiManager
    {
    public:
        WifiManager(Configuration& config, Logger& logger)
            : _config(config),
              _logger(logger) { }


        /// @brief Initialize the WiFi connection. Should be called once during setup.
        /// @return true if the WiFi connection was established, false otherwise
        virtual bool setup() = 0;

        /// @brief Check WiFi connection status and attempt reconnect if disconnected. Should be called regularly in the main loop.
        /// @return true if the WiFi connection is established, false otherwise
        virtual bool loop() = 0;

        /// @brief Trigger a WiFi reconnect attempt. Should be used after configuration changes.
        /// @return true if the WiFi connection was established, false otherwise
        virtual bool reconnect() = 0;

        /// @brief Check if the WiFi is currently connected.
        /// @return true if the WiFi connection is established, false otherwise
        virtual bool isConnected() const = 0;

    protected:
        Configuration& _config;
        Logger& _logger;
    };
}

#endif