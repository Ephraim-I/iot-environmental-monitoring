#include "TelemetryBuffer.h"

TelemetryBuffer::TelemetryBuffer()
    : _head(0),
      _tail(0),
      _count(0) {
}

bool TelemetryBuffer::enqueue(const String& payload) {
    if (isFull()) {
        return false;
    }

    if (payload.length() > TELEMETRY_BUFFER_MAX_PAYLOAD_SIZE) {
        return false;
    }

    _buffer[_tail] = payload;

    _tail = (_tail + 1) % TELEMETRY_BUFFER_CAPACITY;
    _count++;

    return true;
}

bool TelemetryBuffer::dequeue(String& payload) {
    if (isEmpty()) {
        return false;
    }

    payload = _buffer[_head];
    _buffer[_head] = "";

    _head = (_head + 1) % TELEMETRY_BUFFER_CAPACITY;
    _count--;

    return true;
}

bool TelemetryBuffer::peek(String& payload) const {
    if (isEmpty()) {
        return false;
    }

    payload = _buffer[_head];

    return true;
}

bool TelemetryBuffer::isEmpty() const {
    return _count == 0;
}

bool TelemetryBuffer::isFull() const {
    return _count >= TELEMETRY_BUFFER_CAPACITY;
}

uint8_t TelemetryBuffer::size() const {
    return _count;
}

uint8_t TelemetryBuffer::capacity() const {
    return TELEMETRY_BUFFER_CAPACITY;
}
