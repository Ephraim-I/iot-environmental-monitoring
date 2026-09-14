#include <Arduino.h>
#include <unity.h>

#include <Sensor.h>
#include <Measurement.h>
#include <MeasurementConfig.h>

class MultiMeasurementSensor : public Sensor {
public:
    bool begin() override {
        return true;
    }

    size_t measurementCount() const override {
        return 4;
    }

    bool read(
        Measurement* measurements,
        size_t maxMeasurements
    ) override {

        if (measurements == nullptr ||
            maxMeasurements < measurementCount()) {
            return false;
        }

        measurements[0] = {
            "temperature",
            27.5f,
            "C",
            true
        };

        measurements[1] = {
            "humidity",
            61.0f,
            "%RH",
            true
        };

        measurements[2] = {
            "pressure",
            1012.4f,
            "hPa",
            true
        };

        measurements[3] = {
            "co2",
            540.0f,
            "ppm",
            true
        };

        return true;
    }
};

void test_sensor_reports_multiple_measurements() {
    MultiMeasurementSensor sensor;

    TEST_ASSERT_TRUE(sensor.begin());
    TEST_ASSERT_EQUAL_UINT(4, sensor.measurementCount());
}

void test_sensor_fills_multiple_measurements() {
    MultiMeasurementSensor sensor;

    Measurement measurements[
        MAX_MEASUREMENTS_PER_SAMPLE
    ];

    TEST_ASSERT_TRUE(sensor.begin());

    TEST_ASSERT_TRUE(
        sensor.read(
            measurements,
            MAX_MEASUREMENTS_PER_SAMPLE
        )
    );

    TEST_ASSERT_EQUAL_STRING(
        "temperature",
        measurements[0].name
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        27.5f,
        measurements[0].value
    );

    TEST_ASSERT_EQUAL_STRING(
        "humidity",
        measurements[1].name
    );

    TEST_ASSERT_EQUAL_STRING(
        "pressure",
        measurements[2].name
    );

    TEST_ASSERT_EQUAL_STRING(
        "co2",
        measurements[3].name
    );
}

void test_sensor_rejects_insufficient_buffer() {
    MultiMeasurementSensor sensor;

    Measurement measurements[3];

    TEST_ASSERT_TRUE(sensor.begin());

    TEST_ASSERT_FALSE(
        sensor.read(
            measurements,
            3
        )
    );
}

void test_measurement_identity_is_not_position_dependent() {
    Measurement measurements[4] = {
        {
            "humidity",
            55.0f,
            "%RH",
            true
        },
        {
            "co2",
            500.0f,
            "ppm",
            true
        },
        {
            "temperature",
            26.0f,
            "C",
            true
        },
        {
            "pressure",
            1010.0f,
            "hPa",
            true
        }
    };

    TEST_ASSERT_EQUAL_STRING(
        "humidity",
        measurements[0].name
    );

    TEST_ASSERT_EQUAL_STRING(
        "temperature",
        measurements[2].name
    );

    TEST_ASSERT_EQUAL_STRING(
        "co2",
        measurements[1].name
    );
}

void test_measurement_capacity_supports_four_values() {
    TEST_ASSERT_GREATER_OR_EQUAL_UINT(
        4,
        MAX_MEASUREMENTS_PER_SAMPLE
    );
}

void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(
        test_sensor_reports_multiple_measurements
    );

    RUN_TEST(
        test_sensor_fills_multiple_measurements
    );

    RUN_TEST(
        test_sensor_rejects_insufficient_buffer
    );

    RUN_TEST(
        test_measurement_identity_is_not_position_dependent
    );

    RUN_TEST(
        test_measurement_capacity_supports_four_values
    );

    UNITY_END();
}

void loop() {
}

