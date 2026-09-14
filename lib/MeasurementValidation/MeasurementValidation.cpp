#include <cstring>

#include "MeasurementValidation.h"

bool validateMeasurement(
    const Measurement& measurement,
    const MeasurementValidationRule& rule
) {
    if (!measurement.valid) {
        return false;
    }

    if (measurement.name == nullptr ||
        rule.name == nullptr) {
        return false;
    }

    if (strcmp(measurement.name, rule.name) != 0) {
        return false;
    }

    if (measurement.value < rule.minimum ||
        measurement.value > rule.maximum) {
        return false;
    }

    return true;
}
