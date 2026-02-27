#ifndef NTP_CLOCK_SERVICE_H
#define NTP_CLOCK_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "ClockService.h"
#include "Logger.h"
#include "Configuration.h"

namespace smartdevices::clock {

using namespace smartdevices::logging;
using namespace smartdevices::configuration;

class NtpClockService final : public ClockService
{
public:
    NtpClockService(Configuration& config, Logger& logger)
        : _config(config),
          _logger(logger),
          _started(false),
          _timeValid(false),
          _lastCheck(0),
          _checkInterval(5000),
          _lastSyncTrigger(0),
          _resyncInterval(3600000)
    {
    }

    bool setup() override
    {
        char server1[128] = "pool.ntp.org";
        char server2[128] = "time.nist.gov";

        _config.getString("ntp:server1", server1, sizeof(server1));
        _config.getString("ntp:server2", server2, sizeof(server2));

        _server1 = server1;
        _server2 = server2;

        _logger.info("[NTP] Service configured. Servers: %s, %s",
                     _server1.c_str(), _server2.c_str());

        bool valid = _config.getInt("ntp:updateIntervalMs", _resyncInterval);
        if (valid) {
            _logger.info("[NTP] Update interval set to %lu ms", _resyncInterval);
        } else {
            _logger.warn("[NTP] Update interval not set or invalid, using default %lu ms", _resyncInterval);
        }

        return true; 
    }

    bool loop() override
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            _started = false;
            _timeValid = false;
            return false;
        }

        unsigned long nowMs = millis();

        if (!_started)
        {
            startSntp();
            _started = true;
            _lastSyncTrigger = nowMs;
        }

        if (nowMs - _lastCheck >= _checkInterval)
        {
            _lastCheck = nowMs;
            checkTimeValidity();
        }

        if (nowMs - _lastSyncTrigger >= _resyncInterval)
        {
            _logger.info("[NTP] Periodic SNTP re-sync.");
            startSntp();
            _lastSyncTrigger = nowMs;
        }

        return _timeValid;
    }

private:
    Configuration& _config;
    Logger& _logger;

    String _server1;
    String _server2;

    bool _started;
    bool _timeValid;

    unsigned long _lastCheck;
    unsigned long _checkInterval;

    unsigned long _lastSyncTrigger;
    int _resyncInterval;

    void startSntp()
    {
        _logger.info("[NTP] Starting SNTP sync.");

        configTime(0, 0, _server1.c_str(), _server2.c_str());

        _timeValid = false;
    }

    void checkTimeValidity()
    {
        time_t nowTime = time(nullptr);

        if (nowTime > 8 * 3600 * 2)
        {
            if (!_timeValid)
            {
                _timeValid = true;

                struct tm tmInfo;
                gmtime_r(&nowTime, &tmInfo);

                _logger.info("[NTP] Time synchronized: %s", asctime(&tmInfo));
            }
        }
    }
};

} 

#endif