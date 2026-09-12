#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>

struct TelemetryData {
    const char* deviceId;
    const char* firmwareVersion;

    unsigned long uptimeMs;

    float temperature;
    float humidity;

    int wifiRssi;

    const char* healthState;
};

String telemetryToJson(const TelemetryData& data);

#endif
