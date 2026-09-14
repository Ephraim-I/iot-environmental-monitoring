#ifndef SAMPLER_H
#define SAMPLER_H

#include <Arduino.h>
#include <Sensor.h>
#include <SamplingConfig.h>

enum class SampleResult {
    NOT_DUE,
    SUCCESS,
    FAILED
};

class Sampler {
public:
    Sampler(
        Sensor& sensor,
        const SamplingConfig& config
    );

    bool begin();

    size_t measurementCount() const;

    SampleResult sample(
        Measurement* measurements,
        size_t maxMeasurements
    );

private:
    Sensor& _sensor;
    SamplingConfig _config;
    unsigned long _lastSampleTime;
    bool _started;
};

#endif
