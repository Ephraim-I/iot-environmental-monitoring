#include "dht11_sensor.h"

DHT11Sensor::DHT11Sensor(uint8_t pin)
    : _pin(pin),
      _dht(pin, DHT11),
      _initialized(false) {
}

bool DHT11Sensor::begin() {
    _dht.begin();

    _initialized = true;

    return true;
}

size_t DHT11Sensor::measurementCount() const {
    return 2;
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

    if (maxMeasurements < 2) {
        return false;
    }

    float temperature = _dht.readTemperature();
    float humidity = _dht.readHumidity();

    bool temperatureValid = !isnan(temperature);
    bool humidityValid = !isnan(humidity);

    measurements[0] = {
        "temperature",
        temperature,
        "C",
        temperatureValid
    };

    measurements[1] = {
        "humidity",
        humidity,
        "%RH",
        humidityValid
    };

    return temperatureValid && humidityValid;
}
