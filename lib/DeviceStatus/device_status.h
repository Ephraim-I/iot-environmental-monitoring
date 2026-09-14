#ifndef DEVICE_STATUS_H
#define DEVICE_STATUS_H

enum class SystemStatus {
    BOOTING,
    RUNNING,
    SENSOR_ERROR
};

enum class NetworkStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

#endif