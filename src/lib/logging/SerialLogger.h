#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

#include <Arduino.h>
#include <stdarg.h> 

#include "logger.h"

namespace smartdevices::logging {
    class SerialLogger final : public Logger {
    public:
        explicit SerialLogger(Stream& stream, LogLevel level = LogLevel::INFO) : Logger(level), _stream(stream) {}


        void log(LogLevel level, const char* format, va_list args) {
            _stream.print('[');
            _stream.print(levelToString(level));
            _stream.print("] ");

            char buffer[2048];

            vsnprintf(buffer, sizeof(buffer), format, args);

            _stream.println(buffer);
        }

    private:
        Stream& _stream;

    };
}

#endif
