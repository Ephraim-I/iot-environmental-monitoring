#include "device_health.h"

void resetDeviceHealth(DeviceHealth& health) {
    health.uptimeMs = 0;

    health.systemStatus = DeviceSystemStatus::BOOTING;
    health.networkStatus = DeviceNetworkStatus::DISCONNECTED;

    health.wifiRssi = 0;

    health.degradedEvents = 0;
    health.faultEvents = 0;
    health.recoveryEvents = 0;
    health.healthEvaluations = 0;

    health.healthyDurationMs = 0;
    health.degradedDurationMs = 0;
    health.faultDurationMs = 0;

    health.sensorHealthy = false;
}

DeviceHealthState evaluateDeviceHealth(
    const DeviceHealth& health
) {
    if (
        health.systemStatus == DeviceSystemStatus::SENSOR_ERROR ||
        !health.sensorHealthy
    ) {
        return DeviceHealthState::FAULT;
    }

    if (
        health.systemStatus == DeviceSystemStatus::RUNNING &&
        health.networkStatus == DeviceNetworkStatus::CONNECTED
    ) {
        return DeviceHealthState::HEALTHY;
    }

    return DeviceHealthState::DEGRADED;
}

const char* getDeviceHealthStateName(
    DeviceHealthState state
) {
    switch (state) {
        case DeviceHealthState::HEALTHY:
            return "HEALTHY";

        case DeviceHealthState::DEGRADED:
            return "DEGRADED";

        case DeviceHealthState::FAULT:
            return "FAULT";

        default:
            return "UNKNOWN";
    }
}

bool hasDeviceHealthStateChanged(
    DeviceHealthState previous,
    DeviceHealthState current
) {
    return previous != current;
}

void updateDeviceHealthCounters(
    DeviceHealth& health,
    DeviceHealthState previous,
    DeviceHealthState current
) {
    if (previous == current) {
        return;
    }

    if (current == DeviceHealthState::DEGRADED) {
        health.degradedEvents++;
    }

    if (current == DeviceHealthState::FAULT) {
        health.faultEvents++;
    }

    if (
        current == DeviceHealthState::HEALTHY &&
        previous != DeviceHealthState::HEALTHY
    ) {
        health.recoveryEvents++;
    }
}

void updateDeviceHealthDuration(
    DeviceHealth& health,
    DeviceHealthState state,
    unsigned long elapsedMs
) {
    switch (state) {
        case DeviceHealthState::HEALTHY:
            health.healthyDurationMs += elapsedMs;
            break;

        case DeviceHealthState::DEGRADED:
            health.degradedDurationMs += elapsedMs;
            break;

        case DeviceHealthState::FAULT:
            health.faultDurationMs += elapsedMs;
            break;

        default:
            break;
    }
}
