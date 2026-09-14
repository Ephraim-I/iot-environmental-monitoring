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

bool Sampler::sample(
    Measurement* measurements,
    size_t maxMeasurements
) {
    if (!_started) {
        return false;
    }

    unsigned long now = millis();

    if ((now - _lastSampleTime) < _config.intervalMs) {
        return false;
    }

    _lastSampleTime = now;

    return _sensor.read(
        measurements,
        maxMeasurements
    );
}
