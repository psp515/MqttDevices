#ifndef SERIALLOGGER_H
#define SERIALLOGGER_H

#include <Arduino.h>
#include "logger.h"

namespace smartdevices::logging {

class SerialLogger final : public Logger {
public:
    explicit SerialLogger(HardwareSerial& serial)
        : _serial(serial) {}

    void begin() {
        while (!_serial);
    }

    void log(LogLevel level, const char* message) override {
        _serial.print('[');
        _serial.print(levelToString(level));
        _serial.print("] ");
        _serial.println(message);
    }

private:
    HardwareSerial& _serial;
};

} // namespace smartdevices::logging

#endif // SERIALLOGGER_H
