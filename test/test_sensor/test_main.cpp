#include <Arduino.h>
#include <unity.h>

#include <Sensor.h>

class FakeSensor : public Sensor {
public:
    bool initialized = false;
    bool shouldFailBegin = false;
    bool shouldFailRead = false;

    bool begin() override {
        if (shouldFailBegin) {
            return false;
        }

        initialized = true;
        return true;
    }

    size_t measurementCount() const override {
        return 2;
    }

    bool read(
        Measurement* measurements,
        size_t maxMeasurements
    ) override {

        if (shouldFailRead || !initialized) {
            return false;
        }

        if (maxMeasurements < 2) {
            return false;
        }

        measurements[0] = {
            "temperature",
            28.5f,
            "C",
            true
        };

        measurements[1] = {
            "humidity",
            55.0f,
            "%RH",
            true
        };

        return true;
    }
};

void test_sensor_initialization_succeeds() {
    FakeSensor sensor;

    TEST_ASSERT_TRUE(sensor.begin());
}

void test_sensor_initialization_failure() {
    FakeSensor sensor;

    sensor.shouldFailBegin = true;

    TEST_ASSERT_FALSE(sensor.begin());
}

void test_sensor_read_produces_multiple_measurements() {
    FakeSensor sensor;
    sensor.begin();

    Measurement measurements[2];

    TEST_ASSERT_TRUE(
        sensor.read(measurements, 2)
    );

    TEST_ASSERT_EQUAL_STRING(
        "temperature",
        measurements[0].name
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        28.5f,
        measurements[0].value
    );

    TEST_ASSERT_EQUAL_STRING(
        "C",
        measurements[0].unit
    );

    TEST_ASSERT_TRUE(
        measurements[0].valid
    );

    TEST_ASSERT_EQUAL_STRING(
        "humidity",
        measurements[1].name
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        55.0f,
        measurements[1].value
    );
}

void test_sensor_respects_max_measurements() {
    FakeSensor sensor;
    sensor.begin();

    Measurement measurements[1];

    TEST_ASSERT_FALSE(
        sensor.read(measurements, 1)
    );
}

void test_sensor_read_fails_before_initialization() {
    FakeSensor sensor;

    Measurement measurements[2];

    TEST_ASSERT_FALSE(
        sensor.read(measurements, 2)
    );
}

void test_sensor_read_failure() {
    FakeSensor sensor;
    sensor.begin();

    sensor.shouldFailRead = true;

    Measurement measurements[2];

    TEST_ASSERT_FALSE(
        sensor.read(measurements, 2)
    );
}

void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_sensor_initialization_succeeds);
    RUN_TEST(test_sensor_initialization_failure);
    RUN_TEST(test_sensor_read_produces_multiple_measurements);
    RUN_TEST(test_sensor_respects_max_measurements);
    RUN_TEST(test_sensor_read_fails_before_initialization);
    RUN_TEST(test_sensor_read_failure);

    UNITY_END();
}

void loop() {
}
