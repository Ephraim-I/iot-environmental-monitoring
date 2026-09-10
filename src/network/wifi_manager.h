#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password);

    void begin();
    void update();

    bool isConnected();
    String getIPAddress();
    int getRSSI();

private:
    const char* _ssid;
    const char* _password;

    unsigned long _lastReconnectAttempt = 0;

    static constexpr unsigned long RECONNECT_INTERVAL_MS = 10000;
};

#endif