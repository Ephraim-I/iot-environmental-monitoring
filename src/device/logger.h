#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static void info(const char* component, const char* message);
    static void warning(const char* component, const char* message);
    static void error(const char* component, const char* message);

    static void infof(const char* component, const char* format, ...);
    static void warningf(const char* component, const char* format, ...);
    static void errorf(const char* component, const char* format, ...);

private:
    static void log(
        LogLevel level,
        const char* component,
        const char* message
    );

    static const char* levelToString(LogLevel level);
};

#endif
