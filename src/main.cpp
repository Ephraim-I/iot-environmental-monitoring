#include <Arduino.h>

#include "config/device_config.h"
#include "config/pins.h"
#include "config/network_config.local.h"

#include "device/system_status.h"
#include "device/logger.h"

#include "sensors/dht11_sensor.h"

#include "telemetry/telemetry.h"
#include "telemetry/retry_policy.h"

#include "network/wifi_manager.h"
#include "network/http_client.h"

#include <TelemetryBuffer.h>
#include <telemetry_metrics.h>


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

bool telemetryRetryActive = false;
uint8_t telemetryRetryAttempt = 0;
unsigned long telemetryRetryNextAttempt = 0;

unsigned long lastMetricsReport = 0;

#define METRICS_REPORT_INTERVAL_MS 30000

HttpResult transmissionResult = HttpResult::TRANSPORT_ERROR;


TelemetryBuffer telemetryBuffer;
TelemetryMetrics telemetryMetrics;

bool processTelemetryQueue();

float getBufferDropRate(const TelemetryMetrics& metrics);

bool processTelemetryQueue() {
    if (!wifiManager.isConnected()) {
        return false;
    }

    if (telemetryBuffer.isEmpty()) {
        return false;
    }

    String payload;

    if (!telemetryBuffer.peek(payload)) {
        return false;
    }

    Logger::infof(
        "Telemetry",
        "Replaying queued telemetry. Queue size: %u/%u",
        telemetryBuffer.size(),
        telemetryBuffer.capacity()
    );

    HttpResult result =
        telemetryClient.postJson(payload);

    if (result == HttpResult::TRANSPORT_ERROR) {
        telemetryMetrics.transportFailures++;
    }

    if (result == HttpResult::SUCCESS) {
        String transmittedPayload;

        telemetryBuffer.dequeue(transmittedPayload);

        telemetryMetrics.successfulTransmissions++;

        Logger::infof(
            "Telemetry",
            "Queued telemetry transmitted successfully. Queue size: %u/%u",
            telemetryBuffer.size(),
            telemetryBuffer.capacity()
        );

        return true;
    }

    Logger::warning(
        "Telemetry",
        "Queued telemetry transmission failed. Payload retained"
    );

    return false;
}

void reportTelemetryMetrics() {
    float successRate =
        getTransmissionSuccessRate(telemetryMetrics);

    float failureRate =
        getTransmissionFailureRate(telemetryMetrics);

    float dropRate =
        getBufferDropRate(telemetryMetrics);

    Logger::infof(
        "Metrics",
        "TX success: %.2f%% | TX failure: %.2f%% | Buffer drop: %.2f%%",
        successRate,
        failureRate,
        dropRate
    );

    Logger::infof(
        "Metrics",
        "Success: %lu | Failures: %lu | Retries: %lu | Exhaustions: %lu",
        telemetryMetrics.successfulTransmissions,
        telemetryMetrics.transportFailures,
        telemetryMetrics.retryAttempts,
        telemetryMetrics.retryExhaustions
    );

    Logger::infof(
        "Metrics",
        "Buffered: %lu | Dropped: %lu | Queue: %u/%u",
        telemetryMetrics.bufferedPayloads,
        telemetryMetrics.droppedPayloads,
        telemetryBuffer.size(),
        telemetryBuffer.capacity()
    );
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    delay(1000);

    Logger::info("System", "IoT Engineering Lab");
    Logger::info("System", "Environment Sensor Node");

    environmentSensor.begin();

    wifiManager.begin();

    resetTelemetryMetrics(telemetryMetrics);

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

    if (currentTime - lastMetricsReport >= METRICS_REPORT_INTERVAL_MS) {
        lastMetricsReport = currentTime;

        reportTelemetryMetrics();
    }

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

    if (!telemetryRetryActive && !telemetryBuffer.isEmpty()) {
        processTelemetryQueue();
        return;
    }

    /*
     * Process pending telemetry retry.
     *
     * The retry is scheduled using millis() so the
     * firmware does not block while waiting.
     */

    if (telemetryRetryActive) {
        if ((long)(currentTime - telemetryRetryNextAttempt) >= 0) {
            if (!wifiManager.isConnected()) {
                Logger::warning(
                    "Telemetry",
                    "Retry delayed: Wi-Fi unavailable"
                );

                telemetryRetryNextAttempt =
                    currentTime + TELEMETRY_RETRY_DELAY_MS;

                return;
            }

            String payload;

            if (!telemetryBuffer.peek(payload)) {
                Logger::warning(
                    "Telemetry",
                    "Retry requested but buffer is empty"
                );

                telemetryRetryActive = false;
                telemetryRetryAttempt = 0;

                return;
            }

            telemetryRetryAttempt++;

            telemetryMetrics.retryAttempts++;

            Logger::infof(
                "Telemetry",
                "Transmission attempt %u/%u",
                telemetryRetryAttempt,
                TELEMETRY_MAX_ATTEMPTS
            );

            transmissionResult =
                telemetryClient.postJson(payload);

            if (transmissionResult == HttpResult::TRANSPORT_ERROR) {
                telemetryMetrics.transportFailures++;
            }          

            if (transmissionResult == HttpResult::SUCCESS) {
                Logger::info(
                    "Telemetry",
                    "Transmission successful"
                );

                String discardedPayload;

                telemetryBuffer.dequeue(discardedPayload);

                telemetryMetrics.successfulTransmissions++;

                telemetryRetryActive = false;
                telemetryRetryAttempt = 0;
            }
            else if (
                transmissionResult == HttpResult::TRANSPORT_ERROR &&
                telemetryRetryAttempt < TELEMETRY_MAX_ATTEMPTS
            ) {
                telemetryRetryNextAttempt =
                    currentTime + TELEMETRY_RETRY_DELAY_MS;

                Logger::warningf(
                    "Telemetry",
                    "Transport failure. Retrying in %lu ms",
                    TELEMETRY_RETRY_DELAY_MS
                );
            }
            else {
                telemetryMetrics.retryExhaustions++;

                Logger::error(
                    "Telemetry",
                    "Transmission failed after maximum attempts"
                );
            
                telemetryRetryActive = false;
                telemetryRetryAttempt = 0;
            
                Logger::warningf(
                    "Telemetry",
                    "Payload retained in buffer. Queue size: %u/%u",
                    telemetryBuffer.size(),
                    telemetryBuffer.capacity()
                );
            }
        }

        return;
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
     * Store telemetry payload for transmission.
     */
    if (!telemetryBuffer.enqueue(payload)) {
        telemetryMetrics.droppedPayloads++;

        Logger::error(
            "Telemetry",
            "Telemetry buffer full. Payload dropped"
        );
    }
    else {
        telemetryMetrics.bufferedPayloads++;

        Logger::infof(
            "Telemetry",
            "Telemetry buffered. Queue size: %u/%u",
            telemetryBuffer.size(),
            telemetryBuffer.capacity()
        );
    }

    if (!telemetryRetryActive &&
        wifiManager.isConnected() &&
        !telemetryBuffer.isEmpty()) {

        String queuedPayload;

        if (telemetryBuffer.peek(queuedPayload)) {
            telemetryRetryAttempt = 1;

            Logger::infof(
                "Telemetry",
                "Transmission attempt %u/%u",
                telemetryRetryAttempt,
                TELEMETRY_MAX_ATTEMPTS
            );

            transmissionResult =
                telemetryClient.postJson(queuedPayload);

            if (transmissionResult == HttpResult::SUCCESS) {
                Logger::info(
                    "Telemetry",
                    "Transmission successful"
                );

                String transmittedPayload;
                telemetryBuffer.dequeue(transmittedPayload);

                telemetryMetrics.successfulTransmissions++;

                telemetryRetryAttempt = 0;
            }
            else if (
                transmissionResult == HttpResult::TRANSPORT_ERROR &&
                telemetryRetryAttempt < TELEMETRY_MAX_ATTEMPTS
            ) {
                telemetryRetryActive = true;

                telemetryRetryNextAttempt =
                    millis() + TELEMETRY_RETRY_DELAY_MS;

                Logger::warningf(
                    "Telemetry",
                    "Transport failure. Retrying in %lu ms",
                    TELEMETRY_RETRY_DELAY_MS
                );
            }
            else {
                Logger::error(
                    "Telemetry",
                    "Transmission failed"
                );

                telemetryRetryAttempt = 0;

                Logger::warningf(
                    "Telemetry",
                    "Payload retained in buffer. Queue size: %u/%u",
                    telemetryBuffer.size(),
                    telemetryBuffer.capacity()
                );
            }
        }
    }
    else if (!wifiManager.isConnected()) {
        Logger::warning(
            "Telemetry",
            "Transmission deferred: Wi-Fi unavailable"
        );
    }
}