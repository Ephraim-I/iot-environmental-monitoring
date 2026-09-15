#include <Arduino.h>
#include <unity.h>

#include <MeasurementLookup.h>

void test_finds_measurement_by_name() {
    Measurement measurements[3] = {
        {
            "humidity",
            55.0f,
            "%RH",
            true
        },
        {
            "temperature",
            27.0f,
            "C",
            true
        },
        {
            "pressure",
            1012.0f,
            "hPa",
            true
        }
    };

    const Measurement* result = findMeasurement(
        measurements,
        3,
        "temperature"
    );

    TEST_ASSERT_NOT_NULL(result);

    TEST_ASSERT_EQUAL_STRING(
        "temperature",
        result->name
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        27.0f,
        result->value
    );
}

void test_measurement_lookup_does_not_depend_on_position() {
    Measurement measurements[3] = {
        {
            "pressure",
            1012.0f,
            "hPa",
            true
        },
        {
            "temperature",
            27.0f,
            "C",
            true
        },
        {
            "humidity",
            55.0f,
            "%RH",
            true
        }
    };

    const Measurement* temperature =
        findMeasurement(
            measurements,
            3,
            "temperature"
        );

    const Measurement* humidity =
        findMeasurement(
            measurements,
            3,
            "humidity"
        );

    TEST_ASSERT_NOT_NULL(temperature);
    TEST_ASSERT_NOT_NULL(humidity);

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        27.0f,
        temperature->value
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        55.0f,
        humidity->value
    );
}

void test_missing_measurement_returns_null() {
    Measurement measurements[2] = {
        {
            "temperature",
            27.0f,
            "C",
            true
        },
        {
            "humidity",
            55.0f,
            "%RH",
            true
        }
    };

    const Measurement* result =
        findMeasurement(
            measurements,
            2,
            "pressure"
        );

    TEST_ASSERT_NULL(result);
}

void test_null_arguments_are_rejected() {
    Measurement measurements[1] = {
        {
            "temperature",
            27.0f,
            "C",
            true
        }
    };

    TEST_ASSERT_NULL(
        findMeasurement(
            static_cast<Measurement*>(nullptr),
            1,
            "temperature"
        )
    );

    TEST_ASSERT_NULL(
        findMeasurement(
            measurements,
            1,
            nullptr
        )
    );
}

void test_empty_measurement_list_returns_null() {
    Measurement measurements[1] = {
        {
            "temperature",
            27.0f,
            "C",
            true
        }
    };

    TEST_ASSERT_NULL(
        findMeasurement(
            measurements,
            0,
            "temperature"
        )
    );
}

void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_finds_measurement_by_name);
    RUN_TEST(test_measurement_lookup_does_not_depend_on_position);
    RUN_TEST(test_missing_measurement_returns_null);
    RUN_TEST(test_null_arguments_are_rejected);
    RUN_TEST(test_empty_measurement_list_returns_null);

    UNITY_END();
}

void loop() {
}

