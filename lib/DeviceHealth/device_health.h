#ifndef DEVICE_HEALTH_H
#define DEVICE_HEALTH_H

#include <Arduino.h>

enum class DeviceSystemStatus {
    BOOTING,
    RUNNING,
    SENSOR_ERROR
};

enum class DeviceNetworkStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

enum class DeviceHealthState {
    HEALTHY,
    DEGRADED,
    FAULT
};

struct DeviceHealth {
    unsigned long uptimeMs;
    
    DeviceSystemStatus systemStatus;
    DeviceNetworkStatus networkStatus;
    
    int wifiRssi;
    
    bool sensorHealthy;

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
    const DeviceHealth& health
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