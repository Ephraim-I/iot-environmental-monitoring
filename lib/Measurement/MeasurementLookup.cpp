#include <cstring>

#include "MeasurementLookup.h"

const Measurement* findMeasurement(
    const Measurement* measurements,
    size_t count,
    const char* name
) {
    if (measurements == nullptr ||
        name == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < count; i++) {
        if (measurements[i].name == nullptr) {
            continue;
        }

        if (strcmp(measurements[i].name, name) == 0) {
            return &measurements[i];
        }
    }

    return nullptr;
}

Measurement* findMeasurement(
    Measurement* measurements,
    size_t count,
    const char* name
) {
    if (measurements == nullptr ||
        name == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < count; i++) {
        if (measurements[i].name == nullptr) {
            continue;
        }

        if (strcmp(
                measurements[i].name,
                name
            ) == 0) {
            return &measurements[i];
        }
    }

    return nullptr;
}
