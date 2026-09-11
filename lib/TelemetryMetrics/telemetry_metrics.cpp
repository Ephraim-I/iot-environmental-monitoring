#include "telemetry_metrics.h"

void resetTelemetryMetrics(TelemetryMetrics& metrics) {
    metrics.successfulTransmissions = 0;
    metrics.transportFailures = 0;
    metrics.retryAttempts = 0;
    metrics.retryExhaustions = 0;
    metrics.bufferedPayloads = 0;
    metrics.droppedPayloads = 0;
}

float getTransmissionSuccessRate(const TelemetryMetrics& metrics) {
    unsigned long totalAttempts =
        metrics.successfulTransmissions +
        metrics.transportFailures;

    if (totalAttempts == 0) {
        return 0.0f;
    }

    return (
        static_cast<float>(metrics.successfulTransmissions) /
        static_cast<float>(totalAttempts)
    ) * 100.0f;
}

float getTransmissionFailureRate(const TelemetryMetrics& metrics) {
    unsigned long totalAttempts =
        metrics.successfulTransmissions +
        metrics.transportFailures;

    if (totalAttempts == 0) {
        return 0.0f;
    }

    return (
        static_cast<float>(metrics.transportFailures) /
        static_cast<float>(totalAttempts)
    ) * 100.0f;
}

float getBufferDropRate(const TelemetryMetrics& metrics) {
    unsigned long totalBufferAttempts =
        metrics.bufferedPayloads +
        metrics.droppedPayloads;

    if (totalBufferAttempts == 0) {
        return 0.0f;
    }

    return (
        static_cast<float>(metrics.droppedPayloads) /
        static_cast<float>(totalBufferAttempts)
    ) * 100.0f;
}