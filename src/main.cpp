#include <Arduino.h>

#include "config/device_config.h"
#include "config/pins.h"
#include "config/network_config.h"

#include "device/system_status.h"
#include "device/logger.h"

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

    Logger::info("System", "IoT Engineering Lab");
    Logger::info("System", "Environment Sensor Node");

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

        Logger::warning(
            "WiFi",
            "Wi-Fi disconnected"
        );
    }

    Logger::info(
        "Sensor",
        "DHT11 sensor initialized"
    );

    systemStatus.setStatus(SystemStatus::RUNNING);

    Logger::infof(
        "System",
        "System status: %s",
        systemStatus.getStatusName()
    );

    Logger::infof(
        "Network",
        "Network status: %s",
        systemStatus.getNetworkStatusName()
    );
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

        Logger::error(
            "Sensor",
            "Failed to read DHT11 sensor"
        );

        Logger::errorf(
            "System",
            "System status: %s",
            systemStatus.getStatusName()
        );

        return;
    }

    /*
     * Sensor recovered after a previous error.
     */
    systemStatus.setStatus(
        SystemStatus::RUNNING
    );

    digitalWrite(STATUS_LED_PIN, HIGH);

    Logger::infof(
        "Sensor",
        "Temperature: %.1f C",
        reading.temperature
    );

    Logger::infof(
        "Sensor",
        "Humidity: %.1f %%",
        reading.humidity
    );

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

    Logger::infof(
        "Telemetry",
        "Payload: %s",
        payload.c_str()
    );

    /*
     * Only attempt HTTP transmission
     * when Wi-Fi is currently available.
     */
    HttpResult transmissionResult = HttpResult::TRANSPORT_ERROR;

    if (wifiManager.isConnected()) {
        transmissionResult = telemetryClient.postJson(payload);
    } else {
        Logger::warning(
            "Telemetry",
            "Transmission skipped: Wi-Fi unavailable"
        );
    }

    /*
     * Report transmission result.
     */
    switch (transmissionResult) {
        case HttpResult::SUCCESS:
            Logger::info(
                "Telemetry",
                "Transmission successful"
            );
            break;

        case HttpResult::CLIENT_INIT_FAILED:
            Logger::error(
                "Telemetry",
                "Transmission failed: HTTP client initialization"
            );
            break;

        case HttpResult::TRANSPORT_ERROR:
            Logger::error(
                "Telemetry",
                "Transmission failed: transport error"
            );
            break;

        case HttpResult::SERVER_REJECTED:
            Logger::error(
                "Telemetry",
                "Transmission failed: server rejected request"
            );
            break;
    }

    if (!wifiManager.isConnected()) {
        Logger::errorf(
            "Network",
            "Network status: %s",
            systemStatus.getNetworkStatusName()
        );
    }
}