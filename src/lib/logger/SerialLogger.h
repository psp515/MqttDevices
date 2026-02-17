#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

#include <Arduino.h>
#include "logger.h"

namespace smartdevices::logging {

class SerialLogger final : public Logger {
public:
    explicit SerialLogger(Stream& stream)
        : _stream(stream) {}

    void log(LogLevel level, const char* message) override {
        _stream.print('[');
        _stream.print(levelToString(level));
        _stream.print("] ");
        _stream.println(message);
    }

private:
    Stream& _stream;
};

}

#endif
