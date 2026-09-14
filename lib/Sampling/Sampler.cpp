#include "Sampler.h"

Sampler::Sampler(
    Sensor& sensor,
    const SamplingConfig& config
)
    : _sensor(sensor),
      _config(config),
      _lastSampleTime(0),
      _started(false) {
}

bool Sampler::begin() {
    if (!_sensor.begin()) {
        return false;
    }

    _lastSampleTime = millis();
    _started = true;

    return true;
}

size_t Sampler::measurementCount() const {
    return _sensor.measurementCount();
}

SampleResult Sampler::sample(
    Measurement* measurements,
    size_t maxMeasurements
) {
    if (!_started) {
        return SampleResult::FAILED;
    }

    unsigned long now = millis();

    if ((now - _lastSampleTime) < _config.intervalMs) {
        return SampleResult::NOT_DUE;
    }

    _lastSampleTime = now;

    if (!_sensor.read(
            measurements,
            maxMeasurements
        )) {
        return SampleResult::FAILED;
    }

    return SampleResult::SUCCESS;
}