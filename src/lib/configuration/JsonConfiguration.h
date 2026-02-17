#ifndef JSON_CONFIGURATION_H
#define JSON_CONFIGURATION_H

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <cstring>

#include "Configuration.h"
#include "Logger.h"

using namespace smartdevices::logging;

namespace smartdevices::configuration {

    class JsonConfiguration final : public Configuration
    {
    public:
        explicit JsonConfiguration(Logger& logger, const char* fileName)
            : Configuration(fileName), _logger(logger) {}

        virtual ~JsonConfiguration() {}

        bool load() override
        {
            if (!LittleFS.begin()) {
                _logger.error("Failed to start filesystem");
                return false;
            }

            File file = LittleFS.open(_fileName, "r");
            if (!file) {
                _logger.error("Failed to open file.");
                return false;
            }

            _doc.clear();
            auto err = deserializeJson(_doc, file);
            file.close();

            if (err) {
                _logger.error("Failed to read json.");
                return false;
            }

            return true;
        }

        bool save() override
        {
            return saveNotSecure();
        }

        bool setValue(const char* path, const char* value) override
        {
            JsonVariant v = resolvePath(path, true);
            if (v.isNull()) return false;

            if (!v.isNull() && !v.is<const char*>()) {
                _logger.error("Value parsing failed for set char*.");
                return false;
            }

            v.set(value);
            return saveNotSecure();
        }

        bool setValue(const char* path, int value) override
        {
            JsonVariant v = resolvePath(path, true);
            if (v.isNull()) return false;

            if (!v.isNull() && !v.is<int>()) {
                _logger.error("Value parsing failed for set int.");
                return false;
            }

            v.set(value);
            return saveNotSecure();
        }

        bool setValue(const char* path, double value) override
        {
            JsonVariant v = resolvePath(path, true);
            if (v.isNull()) return false;

            if (!v.isNull() && !v.is<double>() && !v.is<float>()) {
                _logger.error("Value parsing failed for set double.");
                return false;
            }

            v.set(value);
            return saveNotSecure();
        }

        bool setValue(const char* path, bool value) override
        {
            JsonVariant v = resolvePath(path, true);
            if (v.isNull()) return false;

            if (!v.isNull() && !v.is<bool>()) {
                _logger.error("Value parsing failed for bool.");
                return false;
            }

            v.set(value);
            return saveNotSecure();
        }

        bool getString(const char* path, char* buffer, size_t bufferSize) const override
        {
            JsonVariantConst v = resolvePathConst(path);
            if (v.isNull() || !v.is<const char*>()) {
                _logger.error("Get value parsing failed for char*.");
                return false;
            }

            const char* val = v.as<const char*>();
            if (!val) return false;

            strncpy(buffer, val, bufferSize);
            buffer[bufferSize - 1] = '\0';
            return true;
        }

        bool getInt(const char* path, int& value) const override
        {
            JsonVariantConst v = resolvePathConst(path);
            if (v.isNull() || !v.is<int>()) {
                _logger.error("Get value parsing failed for int.");
                return false;
            }

            value = v.as<int>();
            return true;
        }

        bool getDouble(const char* path, double& value) const override
        {
            JsonVariantConst v = resolvePathConst(path);
            if (v.isNull() || (!v.is<double>() && !v.is<float>())) {
                _logger.error("Get value parsing failed for double.");
                return false;
            }

            value = v.as<double>();
            return true;
        }

        bool getBool(const char* path, bool& value) const override
        {
            JsonVariantConst v = resolvePathConst(path);
            if (v.isNull() || !v.is<bool>()) {
                _logger.error("Get value parsing failed for bool.");
                return false;
            }

            value = v.as<bool>();
            return true;
        }

    private:
        JsonDocument _doc;
        Logger& _logger;

        bool saveNotSecure() {

            File file = LittleFS.open(_fileName, "w");
            if (!file) {
                _logger.error("Failed to open file when saving.");
                return false;
            }

            bool success = serializeJsonPretty(_doc, file) != 0;
            file.close();

            return success;
        }

        JsonVariant resolvePath(const char* path, bool create) {
            JsonVariant current = _doc.as<JsonVariant>();

            char localPath[128];
            strncpy(localPath, path, sizeof(localPath));
            localPath[sizeof(localPath) - 1] = '\0';

            char* token = strtok(localPath, ":");

            while (token)
            {
                char* next = strtok(nullptr, ":");

                if (!next)
                    return current[token];

                if (!current[token].is<JsonObject>())
                {
                    if (!create) {
                        _logger.error("Invalid path.");
                        return JsonVariant();
                    }

                    // v7 compatible way
                    current[token].to<JsonObject>();
                }

                current = current[token];
                token = next;
            }

            _logger.error("Invalid path format.");
            return JsonVariant();
        }


        JsonVariantConst resolvePathConst(const char* path) const
        {
            JsonVariantConst current = _doc.as<JsonVariantConst>();

            char localPath[128];
            strncpy(localPath, path, sizeof(localPath));
            localPath[sizeof(localPath) - 1] = '\0';

            char* token = strtok(localPath, ":");

            while (token)
            {
                current = current[token];

                if (current.isNull()) {
                    _logger.error("Path not found.");
                    return JsonVariantConst();
                }

                token = strtok(nullptr, ":");
            }

            return current;
        }
    };

}

#endif