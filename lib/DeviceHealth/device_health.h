#ifndef DEVICE_HEALTH_H
#define DEVICE_HEALTH_H

#include <Arduino.h>

#include <device_status.h>

enum class DeviceHealthState {
    HEALTHY,
    DEGRADED,
    FAULT
};

struct DeviceHealth {
    unsigned long uptimeMs;

    int wifiRssi;

    bool sensorHealthy;

    bool healthStateInitialized;
    DeviceHealthState previousHealthState;
    unsigned long lastHealthEvaluationMs;

    unsigned long degradedEvents;
    unsigned long faultEvents;
    unsigned long recoveryEvents;
    unsigned long healthEvaluations;

    unsigned long healthyDurationMs;
    unsigned long degradedDurationMs;
    unsigned long faultDurationMs;
};

void resetDeviceHealth(DeviceHealth& health);

DeviceHealthState evaluateDeviceHealth(
    SystemStatus systemStatus,
    NetworkStatus networkStatus,
    bool sensorHealthy
);

void updateDeviceHealthState(
    DeviceHealth& health,
    DeviceHealthState currentState,
    unsigned long currentTime
);

const char* getDeviceHealthStateName(
    DeviceHealthState state
);

bool hasDeviceHealthStateChanged(
    DeviceHealthState previous,
    DeviceHealthState current
);

void updateDeviceHealthCounters(
    DeviceHealth& health,
    DeviceHealthState previous,
    DeviceHealthState current
);

void updateDeviceHealthDuration(
    DeviceHealth& health,
    DeviceHealthState state,
    unsigned long elapsedMs
);

#endif
