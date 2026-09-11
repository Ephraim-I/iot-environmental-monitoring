#include <Arduino.h>
#include <unity.h>

#include "TelemetryBuffer.h"

TelemetryBuffer buffer;

void setUp() {
    buffer = TelemetryBuffer();
}

void tearDown() {
}

void test_buffer_starts_empty() {
    TEST_ASSERT_TRUE(buffer.isEmpty());
    TEST_ASSERT_FALSE(buffer.isFull());
    TEST_ASSERT_EQUAL_UINT8(0, buffer.size());
    TEST_ASSERT_EQUAL_UINT8(TELEMETRY_BUFFER_CAPACITY, buffer.capacity());
}

void test_enqueue_and_peek() {
    TEST_ASSERT_TRUE(buffer.enqueue("payload-A"));

    TEST_ASSERT_FALSE(buffer.isEmpty());
    TEST_ASSERT_EQUAL_UINT8(1, buffer.size());

    String payload;

    TEST_ASSERT_TRUE(buffer.peek(payload));
    TEST_ASSERT_EQUAL_STRING("payload-A", payload.c_str());

    TEST_ASSERT_EQUAL_UINT8(1, buffer.size());
}

void test_fifo_order() {
    TEST_ASSERT_TRUE(buffer.enqueue("payload-A"));
    TEST_ASSERT_TRUE(buffer.enqueue("payload-B"));
    TEST_ASSERT_TRUE(buffer.enqueue("payload-C"));

    String payload;

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("payload-A", payload.c_str());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("payload-B", payload.c_str());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("payload-C", payload.c_str());

    TEST_ASSERT_TRUE(buffer.isEmpty());
}

void test_dequeue_empty_buffer_fails() {
    String payload;

    TEST_ASSERT_FALSE(buffer.dequeue(payload));
    TEST_ASSERT_TRUE(buffer.isEmpty());
}

void test_full_buffer_rejects_new_payload() {
    for (uint8_t i = 0; i < TELEMETRY_BUFFER_CAPACITY; ++i) {
        TEST_ASSERT_TRUE(
            buffer.enqueue(String("payload-") + String(i))
        );
    }

    TEST_ASSERT_TRUE(buffer.isFull());
    TEST_ASSERT_EQUAL_UINT8(
        TELEMETRY_BUFFER_CAPACITY,
        buffer.size()
    );

    TEST_ASSERT_FALSE(
        buffer.enqueue("overflow-payload")
    );

    TEST_ASSERT_EQUAL_UINT8(
        TELEMETRY_BUFFER_CAPACITY,
        buffer.size()
    );
}

void test_circular_wraparound() {
    String payload;

    TEST_ASSERT_TRUE(buffer.enqueue("A"));
    TEST_ASSERT_TRUE(buffer.enqueue("B"));
    TEST_ASSERT_TRUE(buffer.enqueue("C"));
    TEST_ASSERT_TRUE(buffer.enqueue("D"));
    TEST_ASSERT_TRUE(buffer.enqueue("E"));

    TEST_ASSERT_TRUE(buffer.isFull());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("A", payload.c_str());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("B", payload.c_str());

    TEST_ASSERT_TRUE(buffer.enqueue("F"));
    TEST_ASSERT_TRUE(buffer.enqueue("G"));

    TEST_ASSERT_TRUE(buffer.isFull());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("C", payload.c_str());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("D", payload.c_str());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("E", payload.c_str());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("F", payload.c_str());

    TEST_ASSERT_TRUE(buffer.dequeue(payload));
    TEST_ASSERT_EQUAL_STRING("G", payload.c_str());

    TEST_ASSERT_TRUE(buffer.isEmpty());
}

void test_oversized_payload_is_rejected() {
    String oversizedPayload;

    for (
        uint16_t i = 0;
        i <= TELEMETRY_BUFFER_MAX_PAYLOAD_SIZE;
        ++i
    ) {
        oversizedPayload += "X";
    }

    TEST_ASSERT_FALSE(
        buffer.enqueue(oversizedPayload)
    );

    TEST_ASSERT_TRUE(buffer.isEmpty());
}

void setup() {
    delay(1000);

    UNITY_BEGIN();

    RUN_TEST(test_buffer_starts_empty);
    RUN_TEST(test_enqueue_and_peek);
    RUN_TEST(test_fifo_order);
    RUN_TEST(test_dequeue_empty_buffer_fails);
    RUN_TEST(test_full_buffer_rejects_new_payload);
    RUN_TEST(test_circular_wraparound);
    RUN_TEST(test_oversized_payload_is_rejected);

    UNITY_END();
}

void loop() {
}
