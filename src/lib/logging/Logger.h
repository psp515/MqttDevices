#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

namespace smartdevices::logging {

    enum class LogLevel {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3,
        Critical = 4
    };

    class Logger {
    public:
        Logger(LogLevel level) : _level(level) {}
        virtual ~Logger() {}

        virtual void log(LogLevel level, const char* message) = 0;

        // Convenience helpers
        virtual void debug(const char* message) {
            log(LogLevel::Debug, message);
        }

        virtual void info(const char* message) {
            log(LogLevel::Info, message);
        }

        virtual void warn(const char* message) {
            log(LogLevel::Warn, message);
        }

        virtual void error(const char* message) {
            log(LogLevel::Error, message);
        }
    protected:
        LogLevel _level;

        static const char* levelToString(LogLevel level) {
            switch (level) {
                case LogLevel::Debug: 
                    return "DEBUG";
                case LogLevel::Info:  
                    return "INFO";
                case LogLevel::Warn:  
                    return "WARN";
                case LogLevel::Error: 
                    return "ERROR";
                case LogLevel::Critical: 
                    return "Critical";
                default:              
                    return "UNKNOWN";
            }
        }
    };
}

#endif // LOGGER_H