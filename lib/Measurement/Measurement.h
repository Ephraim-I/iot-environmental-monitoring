#ifndef MEASUREMENT_H
#define MEASUREMENT_H

struct Measurement {
    const char* name;
    float value;
    const char* unit;
    bool valid;
};

#endif
