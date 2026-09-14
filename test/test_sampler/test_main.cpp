#include <Arduino.h>
#include <unity.h>

#include <Sampler.h>

class FakeSensor : public Sensor {
public:
    bool initialized = false;
    int readCount = 0;

    bool begin() override {
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

        if (!initialized) {
            return false;
        }

        if (measurements == nullptr || maxMeasurements < 1) {
            return false;
        }

        readCount++;

        measurements[0] = {
            "temperature",
            25.0f,
            "C",
            true
        };

        return true;
    }
};


void test_sampler_begin_initializes_sensor() {
    FakeSensor sensor;

    SamplingConfig config{
        1000
    };

    Sampler sampler(sensor, config);

    TEST_ASSERT_FALSE(sensor.initialized);

    TEST_ASSERT_TRUE(sampler.begin());

    TEST_ASSERT_TRUE(sensor.initialized);
}


void test_sampler_does_not_sample_before_interval() {
    FakeSensor sensor;

    SamplingConfig config{
        1000
    };

    Sampler sampler(sensor, config);

    TEST_ASSERT_TRUE(sampler.begin());

    Measurement measurements[1];

    TEST_ASSERT_FALSE(
        sampler.sample(measurements, 1)
    );

    TEST_ASSERT_EQUAL_INT(
        0,
        sensor.readCount
    );
}


void test_sampler_samples_after_interval() {
    FakeSensor sensor;

    SamplingConfig config{
        50
    };

    Sampler sampler(sensor, config);

    TEST_ASSERT_TRUE(sampler.begin());

    Measurement measurements[1];

    delay(60);

    TEST_ASSERT_TRUE(
        sampler.sample(measurements, 1)
    );

    TEST_ASSERT_EQUAL_INT(
        1,
        sensor.readCount
    );

    TEST_ASSERT_EQUAL_STRING(
        "temperature",
        measurements[0].name
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.01f,
        25.0f,
        measurements[0].value
    );
}


void test_sampler_does_not_sample_twice_within_interval() {
    FakeSensor sensor;

    SamplingConfig config{
        100
    };

    Sampler sampler(sensor, config);

    TEST_ASSERT_TRUE(sampler.begin());

    Measurement measurements[1];

    delay(110);

    TEST_ASSERT_TRUE(
        sampler.sample(measurements, 1)
    );

    TEST_ASSERT_EQUAL_INT(
        1,
        sensor.readCount
    );

    TEST_ASSERT_FALSE(
        sampler.sample(measurements, 1)
    );

    TEST_ASSERT_EQUAL_INT(
        1,
        sensor.readCount
    );
}


void test_sampler_rejects_sample_before_begin() {
    FakeSensor sensor;

    SamplingConfig config{
        100
    };

    Sampler sampler(sensor, config);

    Measurement measurements[1];

    TEST_ASSERT_FALSE(
        sampler.sample(measurements, 1)
    );

    TEST_ASSERT_EQUAL_INT(
        0,
        sensor.readCount
    );
}


void test_sampler_propagates_sensor_read_failure() {

    class FailingSensor : public Sensor {
    public:

        bool begin() override {
            return true;
        }

        size_t measurementCount() const override {
            return 2;
        }

        bool read(
            Measurement*,
            size_t
        ) override {
            return false;
        }
    };

    FailingSensor sensor;

    SamplingConfig config{
        0
    };

    Sampler sampler(sensor, config);

    TEST_ASSERT_TRUE(sampler.begin());

    Measurement measurements[1];

    TEST_ASSERT_FALSE(
        sampler.sample(measurements, 1)
    );
}

void test_sampler_reports_sensor_measurement_count() {
    FakeSensor sensor;

    SamplingConfig config{
        1000
    };

    Sampler sampler(
        sensor,
        config
    );

    TEST_ASSERT_EQUAL_UINT(
        2,
        sampler.measurementCount()
    );
}


void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(
        test_sampler_begin_initializes_sensor
    );

    RUN_TEST(
        test_sampler_does_not_sample_before_interval
    );

    RUN_TEST(
        test_sampler_samples_after_interval
    );

    RUN_TEST(
        test_sampler_does_not_sample_twice_within_interval
    );

    RUN_TEST(
        test_sampler_rejects_sample_before_begin
    );

    RUN_TEST(
        test_sampler_propagates_sensor_read_failure
    );

    RUN_TEST(
        test_sampler_reports_sensor_measurement_count
    );

    UNITY_END();
}


void loop() {
}