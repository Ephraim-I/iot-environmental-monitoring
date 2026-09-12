#include "telemetry_anomaly.h"

namespace {

constexpr float MAX_TEMPERATURE_RATE_C_PER_MINUTE = 20.0f;
constexpr float MAX_HUMIDITY_RATE_PERCENT_PER_MINUTE = 60.0f;

}

TelemetryAnomalyResult detectTelemetryAnomaly(
    float currentTemperature,
    float currentHumidity,
    float previousTemperature,
    float previousHumidity,
    unsigned long elapsedMs
) {
    TelemetryAnomalyResult result{
        false,
        false,
        false
    };

    if (elapsedMs == 0) {
        return result;
    }

    float elapsedMinutes =
        static_cast<float>(elapsedMs) / 60000.0f;

    float temperatureChange =
        fabs(currentTemperature - previousTemperature);

    float humidityChange =
        fabs(currentHumidity - previousHumidity);

    float temperatureRate =
        temperatureChange / elapsedMinutes;

    float humidityRate =
        humidityChange / elapsedMinutes;

    if (temperatureRate > MAX_TEMPERATURE_RATE_C_PER_MINUTE) {
        result.temperatureAnomaly = true;
    }

    if (humidityRate > MAX_HUMIDITY_RATE_PERCENT_PER_MINUTE) {
        result.humidityAnomaly = true;
    }

    result.hasAnomaly =
        result.temperatureAnomaly ||
        result.humidityAnomaly;

    return result;
}
