#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <DataQuality.h>

struct Measurement {
    const char* name;
    float value;
    const char* unit;
    bool valid;
    DataQuality quality;
};

#endif
