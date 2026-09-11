#include "wifi_manager.h"

#include "device/logger.h"

#include <WiFi.h>

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {
}

void WiFiManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    unsigned long startTime = millis();
    int WIFI_CONNECTION_TIMEOUT_MS = 15000;

    Logger::info(
        "WiFi",
        "Connecting to Wi-Fi"
    );

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < WIFI_CONNECTION_TIMEOUT_MS) {
        delay(500);
    }

    if (isConnected()) {
        Logger::info(
            "WiFi",
            "Wi-Fi connected"
        );

        Logger::infof(
            "WiFi",
            "IP address: %s",
            WiFi.localIP().toString().c_str()
        );
    } else {
        Logger::error(
            "WiFi",
            "Wi-Fi connection failed"
        );
    }
}

void WiFiManager::update() {
    if (isConnected()) {
        return;
    }

    unsigned long currentTime = millis();

    if (currentTime - _lastReconnectAttempt < RECONNECT_INTERVAL_MS) {
        return;
    }

    _lastReconnectAttempt = currentTime;

    Logger::warning(
        "WiFi",
        "Wi-Fi disconnected. Attempting reconnection"
    );

    WiFi.disconnect();
    WiFi.begin(_ssid, _password);
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIPAddress() {
    if (!isConnected()) {
        return "0.0.0.0";
    }

    return WiFi.localIP().toString();
}

int WiFiManager::getRSSI() {
    if (!isConnected()) {
        return 0;
    }

    return WiFi.RSSI();
}