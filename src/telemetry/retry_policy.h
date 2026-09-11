#ifndef RETRY_POLICY_H
#define RETRY_POLICY_H

#include <Arduino.h>

struct RetryPolicy {
    uint8_t maxAttempts;
    unsigned long retryDelayMs;
};

#endif
