#ifndef CONFIGURATION_H
#define CONFIGURATION_H

namespace smartdevices::configuration {
    class Configuration
    {
    public:
        explicit Configuration(const char* fileName) : _fileName(fileName) {}
        virtual ~Configuration() {}

        virtual bool load() = 0;
        virtual bool save() = 0;

        virtual bool setValue(const char* path, const char* value) = 0;
        virtual bool setValue(const char* path, int value) = 0;
        virtual bool setValue(const char* path, double value) = 0;
        virtual bool setValue(const char* path, bool value) = 0;

        virtual bool getString(const char* path, char* buffer, size_t bufferSize) const = 0;
        virtual bool getInt(const char* path, int& value) const = 0;
        virtual bool getDouble(const char* path, double& value) const = 0;
        virtual bool getBool(const char* path, bool& value) const = 0;

    protected:
        const char* _fileName;
    };
}

#endif
