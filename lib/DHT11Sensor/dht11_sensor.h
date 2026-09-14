#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

#include <Sensor.h>
#include <DHT11Config.h>

class DHT11Sensor : public Sensor {
public:
    explicit DHT11Sensor(
        const DHT11Config& config
    );

    explicit DHT11Sensor(
        uint8_t pin
    );

    bool begin() override;

    size_t measurementCount() const override;

    bool read(
        Measurement* measurements,
        size_t maxMeasurements
    ) override;

private:
    DHT11Config _config;
    DHT _dht;
    bool _initialized;
};

#endif