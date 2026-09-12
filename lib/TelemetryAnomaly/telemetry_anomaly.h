#ifndef TELEMETRY_ANOMALY_H
#define TELEMETRY_ANOMALY_H

#include <Arduino.h>

struct TelemetryAnomalyResult {
    bool hasAnomaly;
    bool temperatureAnomaly;
    bool humidityAnomaly;
};

TelemetryAnomalyResult detectTelemetryAnomaly(
    float currentTemperature,
    float currentHumidity,
    float previousTemperature,
    float previousHumidity,
    unsigned long elapsedMs
);

#endif
