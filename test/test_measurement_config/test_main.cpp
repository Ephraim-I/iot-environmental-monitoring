#include <Arduino.h>
#include <unity.h>

#include <MeasurementConfig.h>

void test_measurement_capacity_is_positive() {
    TEST_ASSERT_GREATER_THAN(
        0,
        MAX_MEASUREMENTS_PER_SAMPLE
    );
}

void test_measurement_capacity_is_bounded() {
    TEST_ASSERT_LESS_OR_EQUAL(
        16,
        MAX_MEASUREMENTS_PER_SAMPLE
    );
}

void test_measurement_capacity_is_eight() {
    TEST_ASSERT_EQUAL_UINT32(
        8,
        MAX_MEASUREMENTS_PER_SAMPLE
    );
}

void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(
        test_measurement_capacity_is_positive
    );

    RUN_TEST(
        test_measurement_capacity_is_bounded
    );

    RUN_TEST(
        test_measurement_capacity_is_eight
    );

    UNITY_END();
}

void loop() {
}
