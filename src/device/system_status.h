#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

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

class SystemStatusManager {
public:
    void setStatus(SystemStatus status);
    SystemStatus getStatus() const;
    const char* getStatusName() const;

    void setNetworkStatus(NetworkStatus status);
    NetworkStatus getNetworkStatus() const;
    const char* getNetworkStatusName() const;

private:
    SystemStatus _status = SystemStatus::BOOTING;
    NetworkStatus _networkStatus = NetworkStatus::DISCONNECTED;
};

#endif
