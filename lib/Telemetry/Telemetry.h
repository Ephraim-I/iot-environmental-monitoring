#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <Arduino.h>
#include <DataQuality.h>

struct TelemetryData {
    const char* deviceId;
    const char* firmwareVersion;
    unsigned long uptimeMs;

    float temperature;
    const char* temperatureQuality;

    float humidity;
    const char* humidityQuality;

    int wifiRssi;

    const char* healthState;
    bool anomalyDetected;
};

const char* dataQualityToString(
    DataQuality quality
);

String telemetryToJson(const TelemetryData& data);

#endif
