#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

#include <Arduino.h>
#include <stdarg.h> 

#include "logger.h"

namespace smartdevices::logging {

class SerialLogger final : public Logger {
public:
    explicit SerialLogger(Stream& stream, LogLevel level = LogLevel::INFO) : Logger(level), _stream(stream) {}

    void log(LogLevel level, const char* format, ...) {
        _stream.print('[');
        _stream.print(levelToString(level));
        _stream.print("] ");

        char buffer[2048];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        _stream.println(buffer);
    }

private:
    Stream& _stream;

};

}

#endif
