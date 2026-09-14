#ifndef DHT11_CONFIG_H
#define DHT11_CONFIG_H

#include <stdint.h>
#include <SensorConfig.h>
#include <MeasurementDefinition.h>

struct DHT11Config {
    SensorConfig sensor;

    MeasurementDefinition temperature;
    MeasurementDefinition humidity;

    uint8_t pin;
};

const DHT11Config DHT11_DEFAULT_CONFIG = {
    {
        "DHT11",
        "DHT11 Temperature/Humidity Sensor",
        2
    },

    {
        "temperature",
        "C",
        0.0f,
        50.0f
    },

    {
        "humidity",
        "%RH",
        20.0f,
        90.0f
    },

    4
};

#endif
