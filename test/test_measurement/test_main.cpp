#include <Arduino.h>
#include <unity.h>

#include <Measurement.h>


void test_measurement_temperature() {
    Measurement measurement{
        "temperature",
        25.5f,
        "C",
        true
    };

    TEST_ASSERT_EQUAL_STRING(
        "temperature",
        measurement.name
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        25.5f,
        measurement.value
    );

    TEST_ASSERT_EQUAL_STRING(
        "C",
        measurement.unit
    );

    TEST_ASSERT_TRUE(
        measurement.valid
    );
}


void test_measurement_humidity() {
    Measurement measurement{
        "humidity",
        60.0f,
        "%RH",
        true
    };

    TEST_ASSERT_EQUAL_STRING(
        "humidity",
        measurement.name
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        60.0f,
        measurement.value
    );

    TEST_ASSERT_EQUAL_STRING(
        "%RH",
        measurement.unit
    );

    TEST_ASSERT_TRUE(
        measurement.valid
    );
}


void test_measurement_invalid_state() {
    Measurement measurement{
        "temperature",
        0.0f,
        "C",
        false
    };

    TEST_ASSERT_FALSE(
        measurement.valid
    );
}


void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(
        test_measurement_temperature
    );

    RUN_TEST(
        test_measurement_humidity
    );

    RUN_TEST(
        test_measurement_invalid_state
    );

    UNITY_END();
}


void loop() {
}
