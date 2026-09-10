#include "wifi_manager.h"

#include <WiFi.h>

WiFiManager::WiFiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {
}

void WiFiManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    Serial.print("Connecting to Wi-Fi");

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < 15000) {

        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (isConnected()) {
        Serial.println("Wi-Fi connected.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("ERROR: Wi-Fi connection failed.");
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

    Serial.println("Wi-Fi disconnected. Attempting reconnection...");

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