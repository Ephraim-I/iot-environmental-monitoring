#include "dht11_sensor.h"

DHT11Sensor::DHT11Sensor(uint8_t pin)
    : _pin(pin), _dht(pin, DHT11) {
}

void DHT11Sensor::begin() {
    _dht.begin();
}

DHT11Reading DHT11Sensor::read() {
    DHT11Reading reading{};

    reading.temperature = _dht.readTemperature();
    reading.humidity = _dht.readHumidity();

    reading.valid = !isnan(reading.temperature) &&
                    !isnan(reading.humidity);

    return reading;
}
