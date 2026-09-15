#include <Arduino.h>

#include "config/device_config.h"
#include "config/pins.h"
#include "config/network_config.local.h"
#include "config/sampling_config.h"

#include "device/system_status.h"
#include "device/logger.h"

#include <dht11_sensor.h>
#include <DHT11Config.h>

#include <Telemetry.h>
#include <TelemetryDelivery.h>

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
#include <MeasurementValidation.h>

#include <DataQuality.h>

DHT11Sensor environmentSensor(
    DHT11_DEFAULT_CONFIG
);

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

unsigned long lastMetricsReport = 0;
unsigned long lastHealthReport = 0;

bool hasPreviousTelemetry = false;

float previousTemperature = 0.0f;
float previousHumidity = 0.0f;

unsigned long previousTelemetryTime = 0;

#define METRICS_REPORT_INTERVAL_MS 30000
#define HEALTH_REPORT_INTERVAL_MS 30000

TelemetryBuffer telemetryBuffer;

TelemetryDelivery telemetryDelivery(
    telemetryBuffer,
    telemetryClient,
    1000,
    10000
);

TelemetryMetrics telemetryMetrics;
DeviceHealth deviceHealth;

float getBufferDropRate(const TelemetryMetrics& metrics);

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

    /*
     * Maintain Wi-Fi connection.
     */
    wifiManager.update();

    if (wifiManager.isConnected()) {
        DeliveryResult deliveryResult =
            telemetryDelivery.process();

        if (deliveryResult == DeliveryResult::SUCCESS) {
            telemetryMetrics.successfulTransmissions++;
        }
        else if (deliveryResult == DeliveryResult::TRANSPORT_ERROR) {
            telemetryMetrics.transportFailures++;
        }
    }
    
    /*
     * Report telemetry metrics periodically.
     */
    if (
        currentTime - lastMetricsReport >=
        METRICS_REPORT_INTERVAL_MS
    ) {
        lastMetricsReport = currentTime;

        reportTelemetryMetrics();
    }

    /*
     * Report device health periodically.
     */
    if (
        currentTime - lastHealthReport >=
        HEALTH_REPORT_INTERVAL_MS
    ) {
        lastHealthReport = currentTime;

        reportDeviceHealth();
    }

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

    /*
    * Sample environmental sensor.
    */
    Measurement measurements[
        MAX_MEASUREMENTS_PER_SAMPLE
    ];

    SampleResult sampleResult =
        sensorSampler.sample(
            measurements,
            MAX_MEASUREMENTS_PER_SAMPLE
        );

    if (sampleResult == SampleResult::NOT_DUE) {
        return;
    }

    if (sampleResult == SampleResult::FAILED) {
        deviceHealth.sensorHealthy = false;

        Logger::error(
            "Sensor",
            "Sensor sampling failed"
        );

        reportDeviceHealth();

        return;
    }

    const size_t measurementCount =
        sensorSampler.measurementCount();

    Measurement* temperatureMeasurement =
        findMeasurement(
            measurements,
            measurementCount,
            "temperature"
        );

    Measurement* humidityMeasurement =
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

    DataQuality temperatureQuality =
        classifyMeasurementQuality(
            *temperatureMeasurement,
            DHT11_DEFAULT_CONFIG.temperature
        );

    DataQuality humidityQuality =
        classifyMeasurementQuality(
            *humidityMeasurement,
            DHT11_DEFAULT_CONFIG.humidity
        );

    temperatureMeasurement->quality = temperatureQuality;
    humidityMeasurement->quality = humidityQuality;

    if (temperatureQuality == DataQuality::INVALID ||
        humidityQuality == DataQuality::INVALID) {

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

    if (temperatureQuality == DataQuality::SUSPECT ||
        humidityQuality == DataQuality::SUSPECT) {

        Logger::warning(
            "Sensor",
            "Environmental measurement is suspect"
        );
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
        dataQualityToString(temperatureQuality),
        humidity,
        dataQualityToString(humidityQuality),
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

}
