#include <Arduino.h>

#include "config/device_config.h"
#include "config/pins.h"
#include "config/network_config.local.h"
#include "config/sampling_config.h"

#include "device/system_status.h"
#include "device/logger.h"

#include <dht11_sensor.h>

#include <Telemetry.h>
#include <RetryPolicy.h>

#include "network/wifi_manager.h"
#include "network/http_client.h"

#include <TelemetryBuffer.h>
#include <telemetry_metrics.h>

#include <device_health.h>
#include <telemetry_anomaly.h>

#include <Sampler.h>
#include <SamplingConfig.h>

#include <MeasurementConfig.h>
#include <MeasurementLookup.h>

#include <DHT11Validation.h>


DHT11Sensor environmentSensor(DHT11_PIN);

WiFiManager wifiManager(
    WIFI_SSID,
    WIFI_PASSWORD
);

HttpClient telemetryClient(
    TELEMETRY_ENDPOINT
);

SystemStatusManager systemStatus;

SamplingConfig samplingConfig{
    DEFAULT_SAMPLING_INTERVAL_MS
};

Sampler sensorSampler(
    environmentSensor,
    samplingConfig
);

bool telemetryRetryActive = false;
uint8_t telemetryRetryAttempt = 0;
unsigned long telemetryRetryNextAttempt = 0;

bool telemetryReplayBackoffActive = false;
unsigned long telemetryReplayNextAttempt = 0;
uint8_t telemetryReplayFailureCount = 0;

unsigned long lastMetricsReport = 0;
unsigned long lastHealthReport = 0;

bool hasPreviousTelemetry = false;

float previousTemperature = 0.0f;
float previousHumidity = 0.0f;

unsigned long previousTelemetryTime = 0;

#define METRICS_REPORT_INTERVAL_MS 30000
#define HEALTH_REPORT_INTERVAL_MS 30000
#define TELEMETRY_REPLAY_INITIAL_DELAY_MS 1000
#define TELEMETRY_REPLAY_MAX_DELAY_MS 10000

HttpResult transmissionResult = HttpResult::TRANSPORT_ERROR;


TelemetryBuffer telemetryBuffer;
TelemetryMetrics telemetryMetrics;
DeviceHealth deviceHealth;

bool processTelemetryQueue();

float getBufferDropRate(const TelemetryMetrics& metrics);

bool processTelemetryQueue() {
    if (!wifiManager.isConnected()) {
        return false;
    }

    if (telemetryBuffer.isEmpty()) {
        telemetryReplayBackoffActive = false;
        telemetryReplayFailureCount = 0;
        return false;
    }

    unsigned long currentTime = millis();

    /*
     * Respect replay backoff timer.
     */
    if (
        telemetryReplayBackoffActive &&
        currentTime < telemetryReplayNextAttempt
    ) {
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

        telemetryReplayFailureCount++;

        unsigned long retryDelay =
            TELEMETRY_REPLAY_INITIAL_DELAY_MS;

        /*
         * Exponential backoff:
         *
         * Failure 1 -> 1 second
         * Failure 2 -> 2 seconds
         * Failure 3 -> 4 seconds
         * Failure 4 -> 8 seconds
         * Failure 5+ -> 10 seconds maximum
         */
        for (
            uint8_t i = 1;
            i < telemetryReplayFailureCount;
            i++
        ) {
            retryDelay *= 2;

            if (retryDelay >= TELEMETRY_REPLAY_MAX_DELAY_MS) {
                retryDelay = TELEMETRY_REPLAY_MAX_DELAY_MS;
                break;
            }
        }

        telemetryReplayNextAttempt =
            currentTime + retryDelay;

        telemetryReplayBackoffActive = true;

        Logger::warningf(
            "Telemetry",
            "Queued telemetry transmission failed. Payload retained. Next retry in %lu ms",
            retryDelay
        );

        return false;
    }

    if (result == HttpResult::SUCCESS) {
        String transmittedPayload;

        telemetryBuffer.dequeue(transmittedPayload);

        telemetryMetrics.successfulTransmissions++;

        /*
         * Successful replay clears the replay backoff.
         */
        telemetryReplayBackoffActive = false;
        telemetryReplayFailureCount = 0;

        Logger::infof(
            "Telemetry",
            "Queued telemetry transmitted successfully. Queue size: %u/%u",
            telemetryBuffer.size(),
            telemetryBuffer.capacity()
        );

        return true;
    }

    /*
     * Server rejection is not treated as a transport retry.
     */
    Logger::warning(
        "Telemetry",
        "Queued telemetry rejected by server. Payload retained"
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

void reportDeviceHealth() {
    DeviceHealthState currentHealthState =
        evaluateDeviceHealth(
            systemStatus.getStatus(),
            systemStatus.getNetworkStatus(),
            deviceHealth.sensorHealthy
        );

    deviceHealth.healthEvaluations++;

    unsigned long currentTime = millis();

    bool wasHealthStateInitialized = deviceHealth.healthStateInitialized;
    DeviceHealthState previousState = deviceHealth.previousHealthState;

    updateDeviceHealthState(
        deviceHealth,
        currentHealthState,
        currentTime
    );

    if (!wasHealthStateInitialized) {
        Logger::infof(
            "Health",
            "Initial health state: %s",
            getDeviceHealthStateName(currentHealthState)
        );
    }
    else if (hasDeviceHealthStateChanged(
                 previousState,
                 currentHealthState
    )) {

        Logger::warningf(
            "Health",
            "Health state changed: %s -> %s",
            getDeviceHealthStateName(previousState),
            getDeviceHealthStateName(currentHealthState)
        );
    }

    Logger::infof(
        "Health",
        "State: %s | Uptime: %lu ms | System: %s | Network: %s | RSSI: %d dBm | Sensor: %s",
        getDeviceHealthStateName(currentHealthState),
        deviceHealth.uptimeMs,
        systemStatus.getStatusName(),
        systemStatus.getNetworkStatusName(),
        deviceHealth.wifiRssi,
        deviceHealth.sensorHealthy ? "HEALTHY" : "ERROR"
    );

    Logger::infof(
        "Health",
        "Events: Degraded: %lu | Faults: %lu | Recoveries: %lu | Evaluations: %lu",
        deviceHealth.degradedEvents,
        deviceHealth.faultEvents,
        deviceHealth.recoveryEvents,
        deviceHealth.healthEvaluations
    );

    Logger::infof(
        "Health",
        "Duration: Healthy: %lu ms | Degraded: %lu ms | Fault: %lu ms",
        deviceHealth.healthyDurationMs,
        deviceHealth.degradedDurationMs,
        deviceHealth.faultDurationMs
    );
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    delay(1000);

    Logger::info("System", "IoT Engineering Lab");
    Logger::info("System", "Environment Sensor Node");

    wifiManager.begin();

    resetTelemetryMetrics(telemetryMetrics);
    resetDeviceHealth(deviceHealth);

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

    if (!sensorSampler.begin()) {
        deviceHealth.sensorHealthy = false;

        systemStatus.setStatus(
            SystemStatus::SENSOR_ERROR
        );

        Logger::error(
            "Sensor",
            "Failed to initialize sensor"
        );
    }
    else {
        deviceHealth.sensorHealthy = true;
        Logger::info(
            "Sensor",
            "DHT11 sensor initialized"
        );
    }

    if (deviceHealth.sensorHealthy) {
        systemStatus.setStatus(
            SystemStatus::RUNNING
        );
    }

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

    if (currentTime - lastHealthReport >= HEALTH_REPORT_INTERVAL_MS) {
        lastHealthReport = currentTime;
        reportDeviceHealth();
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
        Logger::warning("WiFi", "Wi-Fi disconnected");
    }

    deviceHealth.uptimeMs = currentTime;

    deviceHealth.wifiRssi = wifiManager.getRSSI();

    if (!telemetryRetryActive && !telemetryBuffer.isEmpty()) {
        processTelemetryQueue();
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
    * Sample environmental sensor.
    */
    Measurement measurements[
        MAX_MEASUREMENTS_PER_SAMPLE
    ];

    if (!sensorSampler.sample(
            measurements,
            MAX_MEASUREMENTS_PER_SAMPLE
        )) {
        return;
    }

    const size_t measurementCount =
        sensorSampler.measurementCount();

    const Measurement* temperatureMeasurement =
        findMeasurement(
            measurements,
            measurementCount,
            "temperature"
        );

    const Measurement* humidityMeasurement =
        findMeasurement(
            measurements,
            measurementCount,
            "humidity"
        );

    if (temperatureMeasurement == nullptr ||
        humidityMeasurement == nullptr) {

        deviceHealth.sensorHealthy = false;
        digitalWrite(STATUS_LED_PIN, LOW);
        systemStatus.setStatus(SystemStatus::SENSOR_ERROR);

        Logger::error(
            "Sensor",
            "Required environmental measurements missing"
        );

        Logger::errorf(
            "System",
            "System status: %s",
            systemStatus.getStatusName()
        );

        return;
    }

    if (!validateMeasurement(
            *temperatureMeasurement,
            DHT11_TEMPERATURE_RULE
        ) ||
        !validateMeasurement(
            *humidityMeasurement,
            DHT11_HUMIDITY_RULE
        )) {

        deviceHealth.sensorHealthy = false;
        digitalWrite(STATUS_LED_PIN, LOW);
        systemStatus.setStatus(SystemStatus::SENSOR_ERROR);

        Logger::error(
            "Sensor",
            "Invalid environmental measurement"
        );

        Logger::errorf(
            "System",
            "System status: %s",
            systemStatus.getStatusName()
        );

        return;
    }

    float temperature = temperatureMeasurement->value;
    float humidity = humidityMeasurement->value;

    /*
     * Sensor recovered after a previous error.
     */
    systemStatus.setStatus(
        SystemStatus::RUNNING
    );

    deviceHealth.sensorHealthy = true;

    digitalWrite(STATUS_LED_PIN, HIGH);

    Logger::infof(
        "Sensor",
        "Temperature: %.1f C",
        temperatureMeasurement->value
    );

    Logger::infof(
        "Sensor",
        "Humidity: %.1f %%",
        humidityMeasurement->value
    );

    unsigned long currentTelemetryTime = millis();

    bool anomalyDetected = false;

    if (hasPreviousTelemetry) {
        unsigned long elapsedMs =
            currentTelemetryTime - previousTelemetryTime;

        TelemetryAnomalyResult anomaly =
            detectTelemetryAnomaly(
                temperature,
                humidity,
                previousTemperature,
                previousHumidity,
                elapsedMs
        );

        anomalyDetected = anomaly.hasAnomaly;

        if (anomaly.hasAnomaly) {
            if (anomaly.temperatureAnomaly) {
                Logger::warning(
                    "Anomaly",
                    "Temperature anomaly detected"
                );
            }

            if (anomaly.humidityAnomaly) {
                Logger::warning(
                    "Anomaly",
                    "Humidity anomaly detected"
                );
            }
        } else {
            Logger::info(
                "Anomaly",
                "No anomaly detected"
            );
        }
    }

    previousTemperature = temperature;
    previousHumidity = humidity;
    previousTelemetryTime = currentTelemetryTime;
    hasPreviousTelemetry = true;

    /*
     * Evaluate current device health before
     * creating the telemetry sample.
     */
    DeviceHealthState currentHealthState =
        evaluateDeviceHealth(
            systemStatus.getStatus(),
            systemStatus.getNetworkStatus(),
            deviceHealth.sensorHealthy
        );

    const char* healthState =
        getDeviceHealthStateName(currentHealthState);

    /*
     * Build telemetry object.
     */
    TelemetryData telemetry{
        DEVICE_ID,
        FIRMWARE_VERSION,
        millis(),
        temperature,
        humidity,
        wifiManager.getRSSI(),
        healthState,
        anomalyDetected
    };


    /*
     * Convert telemetry to JSON.
     */

    String payload = telemetryToJson(telemetry);

    Logger::info("Telemetry", payload.c_str());

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
