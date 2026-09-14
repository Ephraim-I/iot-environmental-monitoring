#ifndef MEASUREMENT_DEFINITION_H
#define MEASUREMENT_DEFINITION_H

struct MeasurementDefinition {
    const char* name;
    const char* unit;
    float minimum;
    float maximum;
};

#endif
