#ifndef MEASUREMENT_LOOKUP_H
#define MEASUREMENT_LOOKUP_H

#include <stddef.h>
#include <Measurement.h>

const Measurement* findMeasurement(
    const Measurement* measurements,
    size_t count,
    const char* name
);

Measurement* findMeasurement(
    Measurement* measurements,
    size_t count,
    const char* name
);

#endif
