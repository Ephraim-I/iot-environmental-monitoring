#include <Arduino.h>
#include <unity.h>

#include <telemetry_metrics.h>

void test_zero_metrics() {
    TelemetryMetrics metrics{};
    resetTelemetryMetrics(metrics);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, getTransmissionSuccessRate(metrics));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, getTransmissionFailureRate(metrics));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, getBufferDropRate(metrics));
}

void test_transmission_rates() {
    TelemetryMetrics metrics{};

    metrics.successfulTransmissions = 8;
    metrics.transportFailures = 2;

    TEST_ASSERT_EQUAL_FLOAT(80.0f, getTransmissionSuccessRate(metrics));
    TEST_ASSERT_EQUAL_FLOAT(20.0f, getTransmissionFailureRate(metrics));
}

void test_all_transmissions_successful() {
    TelemetryMetrics metrics{};

    metrics.successfulTransmissions = 10;
    metrics.transportFailures = 0;

    TEST_ASSERT_EQUAL_FLOAT(100.0f, getTransmissionSuccessRate(metrics));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, getTransmissionFailureRate(metrics));
}

void test_all_transmissions_failed() {
    TelemetryMetrics metrics{};

    metrics.successfulTransmissions = 0;
    metrics.transportFailures = 10;

    TEST_ASSERT_EQUAL_FLOAT(0.0f, getTransmissionSuccessRate(metrics));
    TEST_ASSERT_EQUAL_FLOAT(100.0f, getTransmissionFailureRate(metrics));
}

void test_buffer_drop_rate() {
    TelemetryMetrics metrics{};

    metrics.bufferedPayloads = 9;
    metrics.droppedPayloads = 1;

    TEST_ASSERT_EQUAL_FLOAT(10.0f, getBufferDropRate(metrics));
}

void test_no_buffer_drops() {
    TelemetryMetrics metrics{};

    metrics.bufferedPayloads = 10;
    metrics.droppedPayloads = 0;

    TEST_ASSERT_EQUAL_FLOAT(0.0f, getBufferDropRate(metrics));
}

void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_zero_metrics);
    RUN_TEST(test_transmission_rates);
    RUN_TEST(test_all_transmissions_successful);
    RUN_TEST(test_all_transmissions_failed);
    RUN_TEST(test_buffer_drop_rate);
    RUN_TEST(test_no_buffer_drops);

    UNITY_END();
}

void loop() {
}
