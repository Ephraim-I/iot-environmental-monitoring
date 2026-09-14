#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include <stddef.h>

struct SensorConfig {
    const char* type;
    const char* name;
    size_t measurementCount;
};

#endif
