#ifndef TELEMETRY_METRICS_H
#define TELEMETRY_METRICS_H

#include <Arduino.h>

struct TelemetryMetrics {
    unsigned long successfulTransmissions;
    unsigned long transportFailures;
    unsigned long retryAttempts;
    unsigned long retryExhaustions;
    unsigned long bufferedPayloads;
    unsigned long droppedPayloads;
};

void resetTelemetryMetrics(TelemetryMetrics& metrics);

float getTransmissionSuccessRate(const TelemetryMetrics& metrics);

float getTransmissionFailureRate(const TelemetryMetrics& metrics);

float getBufferDropRate(const TelemetryMetrics& metrics);

void logTelemetryMetrics(const TelemetryMetrics& metrics);

#endif
