#ifndef TELEMETRY_DELIVERY_H
#define TELEMETRY_DELIVERY_H

#include <Arduino.h>

#include <TelemetryBuffer.h>
#include <HttpTransport.h>

enum class DeliveryResult {
    NO_ACTION,
    SUCCESS,
    TRANSPORT_ERROR,
    SERVER_REJECTED,
    CLIENT_INIT_FAILED
};

class TelemetryDelivery {
public:
    TelemetryDelivery(
        TelemetryBuffer& buffer,
        HttpTransport& transport,
        unsigned long initialRetryDelayMs,
        unsigned long maxRetryDelayMs
    );

    DeliveryResult process();

    bool isBackoffActive() const;

    unsigned long nextAttemptTime() const;

    uint8_t failureCount() const;

private:
    TelemetryBuffer& _buffer;
    HttpTransport& _transport;

    unsigned long _initialRetryDelayMs;
    unsigned long _maxRetryDelayMs;

    bool _backoffActive;
    unsigned long _nextAttemptTime;
    uint8_t _failureCount;

    unsigned long calculateRetryDelay() const;
};

#endif

