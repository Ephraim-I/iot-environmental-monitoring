#include "system_status.h"

void SystemStatusManager::setStatus(SystemStatus status) {
    _status = status;
}

SystemStatus SystemStatusManager::getStatus() const {
    return _status;
}

const char* SystemStatusManager::getStatusName() const {
    switch (_status) {
        case SystemStatus::BOOTING:
            return "BOOTING";

        case SystemStatus::RUNNING:
            return "RUNNING";

        case SystemStatus::SENSOR_ERROR:
            return "SENSOR_ERROR";

        default:
            return "UNKNOWN";
    }
}

void SystemStatusManager::setNetworkStatus(NetworkStatus status) {
    _networkStatus = status;
}

NetworkStatus SystemStatusManager::getNetworkStatus() const {
    return _networkStatus;
}

const char* SystemStatusManager::getNetworkStatusName() const {
    switch (_networkStatus) {
        case NetworkStatus::DISCONNECTED:
            return "DISCONNECTED";

        case NetworkStatus::CONNECTING:
            return "CONNECTING";

        case NetworkStatus::CONNECTED:
            return "CONNECTED";

        default:
            return "UNKNOWN";
    }
}
