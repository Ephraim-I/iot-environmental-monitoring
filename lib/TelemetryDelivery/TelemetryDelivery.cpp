#include "TelemetryDelivery.h"

TelemetryDelivery::TelemetryDelivery(
    TelemetryBuffer& buffer,
    HttpTransport& transport,
    unsigned long initialRetryDelayMs,
    unsigned long maxRetryDelayMs
)
    : _buffer(buffer),
      _transport(transport),
      _initialRetryDelayMs(initialRetryDelayMs),
      _maxRetryDelayMs(maxRetryDelayMs),
      _backoffActive(false),
      _nextAttemptTime(0),
      _failureCount(0) {
}

DeliveryResult TelemetryDelivery::process() {
    if (_buffer.isEmpty()) {
        _backoffActive = false;
        _failureCount = 0;

        return DeliveryResult::NO_ACTION;
    }

    unsigned long currentTime = millis();

    /*
     * Respect the current retry backoff window.
     */
    if (
        _backoffActive &&
        currentTime < _nextAttemptTime
    ) {
        return DeliveryResult::NO_ACTION;
    }

    String payload;

    /*
     * Inspect the oldest payload without removing it.
     */
    if (!_buffer.peek(payload)) {
        return DeliveryResult::NO_ACTION;
    }

    HttpResult result =
        _transport.postJson(payload);

    if (result == HttpResult::TRANSPORT_ERROR) {
        _failureCount++;

        unsigned long retryDelay =
            calculateRetryDelay();

        _nextAttemptTime =
            currentTime + retryDelay;

        _backoffActive = true;

        return DeliveryResult::TRANSPORT_ERROR;
    }

    if (result == HttpResult::SUCCESS) {
        String transmittedPayload;

        /*
         * Remove the payload only after successful
         * transmission.
         */
        if (!_buffer.dequeue(transmittedPayload)) {
            return DeliveryResult::NO_ACTION;
        }

        _backoffActive = false;
        _failureCount = 0;

        return DeliveryResult::SUCCESS;
    }

    if (result == HttpResult::SERVER_REJECTED) {
        /*
         * Server rejection is not a transport failure.
         * Keep the payload in the queue.
         */
        return DeliveryResult::SERVER_REJECTED;
    }

    /*
     * CLIENT_INIT_FAILED:
     * Keep the payload in the queue.
     */
    return DeliveryResult::CLIENT_INIT_FAILED;
}

bool TelemetryDelivery::isBackoffActive() const {
    return _backoffActive;
}

unsigned long TelemetryDelivery::nextAttemptTime() const {
    return _nextAttemptTime;
}

uint8_t TelemetryDelivery::failureCount() const {
    return _failureCount;
}

unsigned long TelemetryDelivery::calculateRetryDelay() const {
    unsigned long retryDelay =
        _initialRetryDelayMs;

    /*
     * Exponential backoff:
     *
     * Failure 1 -> initial delay
     * Failure 2 -> 2 × initial delay
     * Failure 3 -> 4 × initial delay
     * Failure 4 -> 8 × initial delay
     * ...
     *
     * Never exceed the configured maximum.
     */
    for (
        uint8_t i = 1;
        i < _failureCount;
        i++
    ) {
        retryDelay *= 2;

        if (
            retryDelay >=
            _maxRetryDelayMs
        ) {
            return _maxRetryDelayMs;
        }
    }

    return retryDelay;
}
