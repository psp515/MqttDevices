#pragma once

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <mutex>
#include <stdexcept>
#include <cstring>
#include "Configuration.h"

namespace smartdevices::configuration {

class JsonConfiguration final : public Configuration
{
public:
    explicit JsonConfiguration(const char* fileName, size_t capacity = 4096)
        : Configuration(fileName),
          _capacity(capacity),
          _doc(capacity)
    {
    }

    virtual ~JsonConfiguration() {}

    bool load() override
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!LittleFS.begin())
            throw std::runtime_error("LittleFS mount failed");

        File file = LittleFS.open(_fileName, "r");
        if (!file)
            return false;

        _doc.clear();
        auto err = deserializeJson(_doc, file);
        file.close();

        if (err)
            throw std::runtime_error("JSON deserialization failed");

        return true;
    }

    bool save() override
    {
        std::lock_guard<std::mutex> lock(_mutex);

        saveNotSecure();
    }

    void setValue(const char* path, const char* value) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        JsonVariant v = resolvePath(path, true);

        if (!v.isNull() && !v.is<const char*>())
            throw std::runtime_error("Type mismatch: expected string");

        v.set(value);
        saveNotSecure();
    }

    void setValue(const char* path, int value) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        JsonVariant v = resolvePath(path, true);

        if (!v.isNull() && !v.is<int>())
            throw std::runtime_error("Type mismatch: expected int");

        v.set(value);
        saveNotSecure();
    }

    void setValue(const char* path, double value) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        JsonVariant v = resolvePath(path, true);

        if (!v.isNull() && !v.is<double>())
            throw std::runtime_error("Type mismatch: expected double");

        v.set(value);
        saveNotSecure();
    }

    void setValue(const char* path, bool value) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        JsonVariant v = resolvePath(path, true);

        if (!v.isNull() && !v.is<bool>())
            throw std::runtime_error("Type mismatch: expected bool");

        v.set(value);
        saveNotSecure();
    }

    bool getString(const char* path, char* buffer, size_t bufferSize) const override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        JsonVariantConst v = resolvePathConst(path);

        if (!v.is<const char*>())
            throw std::runtime_error("Type mismatch: expected string");

        const char* val = v.as<const char*>();
        if (!val)
            return false;

        strncpy(buffer, val, bufferSize);
        buffer[bufferSize - 1] = '\0';
        return true;
    }

    bool getInt(const char* path, int& value) const override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        JsonVariantConst v = resolvePathConst(path);

        if (!v.is<int>())
            throw std::runtime_error("Type mismatch: expected int");

        value = v.as<int>();
        return true;
    }

    bool getDouble(const char* path, double& value) const override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        JsonVariantConst v = resolvePathConst(path);

        if (!v.is<double>() && !v.is<float>())
            throw std::runtime_error("Type mismatch: expected double");

        value = v.as<double>();
        return true;
    }

    bool getBool(const char* path, bool& value) const override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        JsonVariantConst v = resolvePathConst(path);

        if (!v.is<bool>())
            throw std::runtime_error("Type mismatch: expected bool");

        value = v.as<bool>();
        return true;
    }

private:
    size_t _capacity;
    mutable std::mutex _mutex;
    DynamicJsonDocument _doc;

    bool saveNotSecure()
    {
        File file = LittleFS.open(_fileName, "w");
        if (!file)
            return false;

        if (serializeJsonPretty(_doc, file) == 0)
        {
            file.close();
            throw std::runtime_error("JSON serialization failed");
        }

        file.close();
        return true;
    }

    JsonVariant resolvePath(const char* path, bool create)
    {
        JsonVariant current = _doc.as<JsonVariant>();
        char localPath[128];
        strncpy(localPath, path, sizeof(localPath));
        localPath[sizeof(localPath) - 1] = '\0';

        char* token = strtok(localPath, ":");

        while (token)
        {
            char* next = strtok(nullptr, ":");

            if (!next)
            {
                return current[token];
            }

            if (!current[token].is<JsonObject>())
            {
                if (!create)
                    throw std::runtime_error("Invalid path");

                current[token] = JsonObject();
            }

            current = current[token];
            token = next;
        }

        throw std::runtime_error("Invalid path format");
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
            if (current.isNull())
                throw std::runtime_error("Path not found");

            token = strtok(nullptr, ":");
        }

        return current;
    }
};

}
