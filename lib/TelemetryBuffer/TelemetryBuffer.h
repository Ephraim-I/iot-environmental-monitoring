#ifndef TELEMETRY_BUFFER_H
#define TELEMETRY_BUFFER_H

#include <Arduino.h>

#define TELEMETRY_BUFFER_CAPACITY 5
#define TELEMETRY_BUFFER_MAX_PAYLOAD_SIZE 256

class TelemetryBuffer {
public:
    TelemetryBuffer();

    bool enqueue(const String& payload);
    bool dequeue(String& payload);
    bool peek(String& payload) const;

    bool isEmpty() const;
    bool isFull() const;

    uint8_t size() const;
    uint8_t capacity() const;

private:
    String _buffer[TELEMETRY_BUFFER_CAPACITY];

    uint8_t _head;
    uint8_t _tail;
    uint8_t _count;
};

#endif
