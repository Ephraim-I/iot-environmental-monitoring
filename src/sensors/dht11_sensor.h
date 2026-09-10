#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

struct DHT11Reading {
    float temperature;
    float humidity;
    bool valid;
};

class DHT11Sensor {
public:
    DHT11Sensor(uint8_t pin);

    void begin();
    DHT11Reading read();

private:
    uint8_t _pin;
    DHT _dht;
};

#endif
