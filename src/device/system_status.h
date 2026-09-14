#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include <device_status.h>

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
