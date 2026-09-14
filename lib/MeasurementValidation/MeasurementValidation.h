#ifndef MEASUREMENT_VALIDATION_H
#define MEASUREMENT_VALIDATION_H

#include <Measurement.h>
#include <MeasurementDefinition.h>

struct MeasurementValidationRule {
    const char* name;
    float minimum;
    float maximum;
};

bool validateMeasurement(
    const Measurement& measurement,
    const MeasurementDefinition& definition
);

bool validateMeasurement(
    const Measurement& measurement,
    const MeasurementValidationRule& rule
);

#endif
