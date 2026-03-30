#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h> 

namespace smartdevices::logging {

    enum class LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3,
        CRITICAL = 4
    };

    class Logger {
    public:
        Logger(LogLevel level) : _level(level) {}
        virtual ~Logger() {}

        virtual void log(LogLevel level, const char* format, va_list args) = 0;

        // Convenience helpers
        virtual void info(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(LogLevel::INFO, format, args);
            va_end(args);
        }

        virtual void warn(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(LogLevel::WARN, format, args);
            va_end(args);
        }

        virtual void error(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(LogLevel::ERROR, format, args);
            va_end(args);
        }

        virtual void critical(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(LogLevel::CRITICAL, format, args);
            va_end(args);
        }

        virtual void debug(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(LogLevel::DEBUG, format, args);
            va_end(args);
        }
    protected:
        LogLevel _level;

        static const char* levelToString(LogLevel level) {
            switch (level) {
                case LogLevel::DEBUG: 
                    return "DEBUG";
                case LogLevel::INFO:  
                    return "INFO";
                case LogLevel::WARN:  
                    return "WARN";
                case LogLevel::ERROR: 
                    return "ERROR";
                case LogLevel::CRITICAL: 
                    return "Critical";
                default:              
                    return "UNKNOWN";
            }
        }
    };
}

#endif // LOGGER_H