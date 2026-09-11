#include "logger.h"
#include <stdarg.h>
#include <stdio.h>

void Logger::info(
    const char* component,
    const char* message
) {
    log(LogLevel::INFO, component, message);
}

void Logger::warning(
    const char* component,
    const char* message
) {
    log(LogLevel::WARNING, component, message);
}

void Logger::error(
    const char* component,
    const char* message
) {
    log(LogLevel::ERROR, component, message);
}

void Logger::infof(
    const char* component,
    const char* format,
    ...
) {
    va_list args;
    va_start(args, format);

    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    log(LogLevel::INFO, component, buffer);
}

void Logger::warningf(
    const char* component,
    const char* format,
    ...
) {
    va_list args;
    va_start(args, format);

    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    log(LogLevel::WARNING, component, buffer);
}

void Logger::errorf(
    const char* component,
    const char* format,
    ...
) {
    va_list args;
    va_start(args, format);

    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    log(LogLevel::ERROR, component, buffer);
}

void Logger::log(
    LogLevel level,
    const char* component,
    const char* message
) {
    Serial.print("[");
    Serial.print(levelToString(level));
    Serial.print("] [");
    Serial.print(component);
    Serial.print("] ");
    Serial.println(message);
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:
            return "INFO";

        case LogLevel::WARNING:
            return "WARN";

        case LogLevel::ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}
