#include "dht11_sensor.h"

DHT11Sensor::DHT11Sensor(
    const DHT11Config& config
)
    : _config(config),
      _dht(config.pin, DHT11),
      _initialized(false) {
}

DHT11Sensor::DHT11Sensor(
    uint8_t pin
)
    : DHT11Sensor(
        DHT11Config{
            DHT11_DEFAULT_CONFIG.sensor,
            DHT11_DEFAULT_CONFIG.temperature,
            DHT11_DEFAULT_CONFIG.humidity,
            pin
        }
    ) {
}

bool DHT11Sensor::begin() {
    _dht.begin();

    _initialized = true;

    return true;
}

size_t DHT11Sensor::measurementCount() const {
    return _config.sensor.measurementCount;
}

bool DHT11Sensor::read(
    Measurement* measurements,
    size_t maxMeasurements
) {
    if (!_initialized) {
        return false;
    }

    if (measurements == nullptr) {
        return false;
    }

    if (maxMeasurements < _config.sensor.measurementCount) {
        return false;
    }

    float temperature = _dht.readTemperature();
    float humidity = _dht.readHumidity();

    bool temperatureValid = !isnan(temperature);
    bool humidityValid = !isnan(humidity);

    measurements[0] = {
        _config.temperature.name,
        temperature,
        _config.temperature.unit,
        temperatureValid,
        temperatureValid
            ? DataQuality::VALID
            : DataQuality::INVALID
    };

    measurements[1] = {
        _config.humidity.name,
        humidity,
        _config.humidity.unit,
        humidityValid,
        humidityValid
            ? DataQuality::VALID
            : DataQuality::INVALID
    };

    return temperatureValid && humidityValid;
}