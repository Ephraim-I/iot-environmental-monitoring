#ifndef SENSOR_H
#define SENSOR_H

#include <stddef.h>
#include <Measurement.h>

class Sensor {
public:
    virtual bool begin() = 0;

    virtual size_t measurementCount() const = 0;

    virtual bool read(
        Measurement* measurements,
        size_t maxMeasurements
    ) = 0;

    virtual ~Sensor() = default;
};

#endif
