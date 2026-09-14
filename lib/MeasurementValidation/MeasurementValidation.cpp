#include "MeasurementValidation.h"

#include <cstring>

bool validateMeasurement(
    const Measurement& measurement,
    const MeasurementDefinition& definition
) {
    if (!measurement.valid) {
        return false;
    }

    if (measurement.name == nullptr ||
        definition.name == nullptr) {
        return false;
    }

    if (strcmp(
            measurement.name,
            definition.name
        ) != 0) {
        return false;
    }

    if (measurement.value < definition.minimum ||
        measurement.value > definition.maximum) {
        return false;
    }

    return true;
}

bool validateMeasurement(
    const Measurement& measurement,
    const MeasurementValidationRule& rule
) {
    MeasurementDefinition definition = {
        rule.name,
        nullptr,
        rule.minimum,
        rule.maximum
    };

    return validateMeasurement(
        measurement,
        definition
    );
}
