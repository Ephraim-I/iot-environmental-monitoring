#include <Arduino.h>
#include <unity.h>

#include <MeasurementValidation.h>
#include <DHT11Validation.h>


void test_valid_dht11_temperature() {
    Measurement measurement{
        "temperature",
        28.5f,
        "C",
        true
    };

    TEST_ASSERT_TRUE(
        validateMeasurement(
            measurement,
            DHT11_TEMPERATURE_RULE
        )
    );
}


void test_invalid_dht11_temperature() {
    Measurement measurement{
        "temperature",
        85.0f,
        "C",
        true
    };

    TEST_ASSERT_FALSE(
        validateMeasurement(
            measurement,
            DHT11_TEMPERATURE_RULE
        )
    );
}


void test_valid_dht11_humidity() {
    Measurement measurement{
        "humidity",
        55.0f,
        "%RH",
        true
    };

    TEST_ASSERT_TRUE(
        validateMeasurement(
            measurement,
            DHT11_HUMIDITY_RULE
        )
    );
}


void test_invalid_dht11_humidity() {
    Measurement measurement{
        "humidity",
        105.0f,
        "%RH",
        true
    };

    TEST_ASSERT_FALSE(
        validateMeasurement(
            measurement,
            DHT11_HUMIDITY_RULE
        )
    );
}


void test_dht11_temperature_lower_boundary() {
    Measurement measurement{
        "temperature",
        -40.0f,
        "C",
        true
    };

    TEST_ASSERT_TRUE(
        validateMeasurement(
            measurement,
            DHT11_TEMPERATURE_RULE
        )
    );
}


void test_dht11_humidity_upper_boundary() {
    Measurement measurement{
        "humidity",
        100.0f,
        "%RH",
        true
    };

    TEST_ASSERT_TRUE(
        validateMeasurement(
            measurement,
            DHT11_HUMIDITY_RULE
        )
    );
}


void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(
        test_valid_dht11_temperature
    );

    RUN_TEST(
        test_invalid_dht11_temperature
    );

    RUN_TEST(
        test_valid_dht11_humidity
    );

    RUN_TEST(
        test_invalid_dht11_humidity
    );

    RUN_TEST(
        test_dht11_temperature_lower_boundary
    );

    RUN_TEST(
        test_dht11_humidity_upper_boundary
    );

    UNITY_END();
}


void loop() {
}

