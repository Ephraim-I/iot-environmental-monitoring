#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

#include <Sensor.h>

class DHT11Sensor : public Sensor {
public:
    explicit DHT11Sensor(uint8_t pin);

    bool begin() override;

    size_t measurementCount() const override;

    bool read(
        Measurement* measurements,
        size_t maxMeasurements
    ) override;

private:
    uint8_t _pin;
    DHT _dht;
    bool _initialized;
};

#endif