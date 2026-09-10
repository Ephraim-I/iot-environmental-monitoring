#include <Arduino.h>

#include "config/device_config.h"
#include "config/pins.h"
#include "config/network_config.h"

#include "device/system_status.h"

#include "sensors/dht11_sensor.h"

#include "telemetry/telemetry.h"

#include "network/wifi_manager.h"
#include "network/http_client.h"


DHT11Sensor environmentSensor(DHT11_PIN);

WiFiManager wifiManager(
    WIFI_SSID,
    WIFI_PASSWORD
);

HttpClient telemetryClient(
    TELEMETRY_ENDPOINT
);

SystemStatusManager systemStatus;

unsigned long lastSensorRead = 0;


void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("IoT Engineering Lab");
    Serial.println("Environment Sensor Node");
    Serial.println("================================");

    environmentSensor.begin();

    wifiManager.begin();

    if (wifiManager.isConnected()) {
        systemStatus.setNetworkStatus(
            NetworkStatus::CONNECTED
        );
    } else {
        systemStatus.setNetworkStatus(
            NetworkStatus::DISCONNECTED
        );

        Serial.println("WARNING: Wi-Fi disconnected.");
    }

    if (wifiManager.isConnected()) {
        Serial.print("Wi-Fi: connected, IP: ");
        Serial.println(wifiManager.getIPAddress());
    }

    Serial.println("DHT11 sensor initialized.");

    systemStatus.setStatus(SystemStatus::RUNNING);

    Serial.print("System status: ");
    Serial.println(systemStatus.getStatusName());

    Serial.print("Network status: ");
    Serial.println(systemStatus.getNetworkStatusName());
}


void loop() {
    unsigned long currentTime = millis();

    /*
     * Maintain Wi-Fi connection.
     */
    wifiManager.update();

    /*
     * Keep network status synchronized
     * with the actual Wi-Fi connection.
     */
    if (wifiManager.isConnected()) {
        systemStatus.setNetworkStatus(
            NetworkStatus::CONNECTED
        );
    } else {
        systemStatus.setNetworkStatus(
            NetworkStatus::DISCONNECTED
        );
    }

    /*
     * Sensor sampling interval.
     */
    if (currentTime - lastSensorRead < SENSOR_READ_INTERVAL_MS) {
        return;
    }

    lastSensorRead = currentTime;

    /*
     * Read environmental sensor.
     */
    DHT11Reading reading = environmentSensor.read();

    if (!reading.valid) {
        digitalWrite(STATUS_LED_PIN, LOW);

        systemStatus.setStatus(
            SystemStatus::SENSOR_ERROR
        );

        Serial.println("ERROR: Failed to read DHT11 sensor.");

        Serial.print("System status: ");
        Serial.println(systemStatus.getStatusName());

        return;
    }

    /*
     * Sensor recovered after a previous error.
     */
    systemStatus.setStatus(
        SystemStatus::RUNNING
    );

    digitalWrite(STATUS_LED_PIN, HIGH);

    Serial.print("Temperature: ");
    Serial.print(reading.temperature, 1);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(reading.humidity, 1);
    Serial.println(" %");

    /*
     * Build telemetry object.
     */
    TelemetryData telemetry{
        DEVICE_ID,
        FIRMWARE_VERSION,
        millis(),
        reading.temperature,
        reading.humidity,
        wifiManager.getRSSI()
    };

    /*
     * Convert telemetry to JSON.
     */
    String payload = telemetryToJson(telemetry);

    Serial.print("Telemetry: ");
    Serial.println(payload);

    /*
     * Only attempt HTTP transmission
     * when Wi-Fi is currently available.
     */
    bool sent = false;

    if (wifiManager.isConnected()) {
        sent = telemetryClient.postJson(payload);
    } else {
        Serial.println(
            "Telemetry transmission skipped: Wi-Fi unavailable."
        );
    }

    /*
     * Report transmission result.
     */
    if (sent) {
        Serial.println(
            "Telemetry transmission: SUCCESS"
        );
    } else {
        Serial.println(
            "Telemetry transmission: FAILED"
        );

        Serial.print("Network status: ");
        Serial.println(
            systemStatus.getNetworkStatusName()
        );
    }

    Serial.println("--------------------------------");
}
