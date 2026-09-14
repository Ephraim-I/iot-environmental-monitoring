#include <Arduino.h>
#include <unity.h>

#include <MeasurementValidation.h>


void test_valid_measurement_within_range() {
    Measurement measurement{
        "temperature",
        25.0f,
        "C",
        true
    };

    MeasurementValidationRule rule{
        "temperature",
        -40.0f,
        85.0f
    };

    TEST_ASSERT_TRUE(
        validateMeasurement(
            measurement,
            rule
        )
    );
}


void test_measurement_below_range() {
    Measurement measurement{
        "temperature",
        -50.0f,
        "C",
        true
    };

    MeasurementValidationRule rule{
        "temperature",
        -40.0f,
        85.0f
    };

    TEST_ASSERT_FALSE(
        validateMeasurement(
            measurement,
            rule
        )
    );
}


void test_measurement_above_range() {
    Measurement measurement{
        "temperature",
        100.0f,
        "C",
        true
    };

    MeasurementValidationRule rule{
        "temperature",
        -40.0f,
        85.0f
    };

    TEST_ASSERT_FALSE(
        validateMeasurement(
            measurement,
            rule
        )
    );
}


void test_invalid_measurement_is_rejected() {
    Measurement measurement{
        "temperature",
        25.0f,
        "C",
        false
    };

    MeasurementValidationRule rule{
        "temperature",
        -40.0f,
        85.0f
    };

    TEST_ASSERT_FALSE(
        validateMeasurement(
            measurement,
            rule
        )
    );
}


void test_wrong_measurement_name_is_rejected() {
    Measurement measurement{
        "humidity",
        50.0f,
        "%RH",
        true
    };

    MeasurementValidationRule rule{
        "temperature",
        -40.0f,
        85.0f
    };

    TEST_ASSERT_FALSE(
        validateMeasurement(
            measurement,
            rule
        )
    );
}


void test_boundary_values_are_accepted() {
    Measurement minimum{
        "temperature",
        -40.0f,
        "C",
        true
    };

    Measurement maximum{
        "temperature",
        85.0f,
        "C",
        true
    };

    MeasurementValidationRule rule{
        "temperature",
        -40.0f,
        85.0f
    };

    TEST_ASSERT_TRUE(
        validateMeasurement(
            minimum,
            rule
        )
    );

    TEST_ASSERT_TRUE(
        validateMeasurement(
            maximum,
            rule
        )
    );
}


void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(
        test_valid_measurement_within_range
    );

    RUN_TEST(
        test_measurement_below_range
    );

    RUN_TEST(
        test_measurement_above_range
    );

    RUN_TEST(
        test_invalid_measurement_is_rejected
    );

    RUN_TEST(
        test_wrong_measurement_name_is_rejected
    );

    RUN_TEST(
        test_boundary_values_are_accepted
    );

    UNITY_END();
}


void loop() {
}
