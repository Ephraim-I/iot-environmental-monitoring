#include <Arduino.h>
#include <unity.h>

#include <dht11_sensor.h>

void test_dht11_sensor_is_a_sensor() {
    DHT11Sensor sensor(4);

    Sensor* genericSensor = &sensor;

    TEST_ASSERT_NOT_NULL(genericSensor);
}

void test_dht11_sensor_begin_succeeds() {
    DHT11Sensor sensor(4);

    TEST_ASSERT_TRUE(sensor.begin());
}

void test_dht11_sensor_rejects_insufficient_measurement_buffer() {
    DHT11Sensor sensor(4);

    sensor.begin();

    Measurement measurements[1];

    TEST_ASSERT_FALSE(
        sensor.read(measurements, 1)
    );
}

void test_dht11_sensor_rejects_null_buffer() {
    DHT11Sensor sensor(4);

    sensor.begin();

    TEST_ASSERT_FALSE(
        sensor.read(nullptr, 2)
    );
}

void test_dht11_sensor_rejects_read_before_initialization() {
    DHT11Sensor sensor(4);

    Measurement measurements[2];

    TEST_ASSERT_FALSE(
        sensor.read(measurements, 2)
    );
}

void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_dht11_sensor_is_a_sensor);
    RUN_TEST(test_dht11_sensor_begin_succeeds);
    RUN_TEST(
        test_dht11_sensor_rejects_insufficient_measurement_buffer
    );
    RUN_TEST(
        test_dht11_sensor_rejects_null_buffer
    );
    RUN_TEST(
        test_dht11_sensor_rejects_read_before_initialization
    );

    UNITY_END();
}

void loop() {
}
