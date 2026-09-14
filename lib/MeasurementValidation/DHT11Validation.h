#ifndef DHT11_VALIDATION_H
#define DHT11_VALIDATION_H

#include <MeasurementValidation.h>

const MeasurementValidationRule DHT11_TEMPERATURE_RULE = {
    "temperature",
    -40.0f,
    80.0f
};

const MeasurementValidationRule DHT11_HUMIDITY_RULE = {
    "humidity",
    0.0f,
    100.0f
};

#endif
