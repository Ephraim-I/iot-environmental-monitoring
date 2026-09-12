#include <Arduino.h>
#include <unity.h>

#include <telemetry_anomaly.h>

void test_normal_telemetry_has_no_anomaly()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            25.0,
            60.0,
            24.8,
            59.5,
            2000
        );

    TEST_ASSERT_FALSE(result.hasAnomaly);
}

void test_temperature_rate_anomaly_is_detected()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            40.0,
            60.0,
            25.0,
            60.0,
            2000
        );

    TEST_ASSERT_TRUE(result.hasAnomaly);
    TEST_ASSERT_TRUE(result.temperatureAnomaly);
    TEST_ASSERT_FALSE(result.humidityAnomaly);
}

void test_humidity_rate_anomaly_is_detected()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            25.0,
            95.0,
            25.0,
            50.0,
            2000
        );

    TEST_ASSERT_TRUE(result.hasAnomaly);
    TEST_ASSERT_FALSE(result.temperatureAnomaly);
    TEST_ASSERT_TRUE(result.humidityAnomaly);
}

void test_multiple_anomalies_are_detected()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            40.0,
            95.0,
            25.0,
            50.0,
            2000
        );

    TEST_ASSERT_TRUE(result.hasAnomaly);
    TEST_ASSERT_TRUE(result.temperatureAnomaly);
    TEST_ASSERT_TRUE(result.humidityAnomaly);
}

void test_small_changes_are_not_anomalies()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            25.5,
            60.5,
            25.0,
            60.0,
            2000
        );

    TEST_ASSERT_FALSE(result.hasAnomaly);
}

void test_exact_temperature_rate_limit_is_not_anomaly()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            35.0,
            60.0,
            25.0,
            60.0,
            30000
        );

    TEST_ASSERT_FALSE(result.temperatureAnomaly);
}

void test_zero_elapsed_time_is_not_processed()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            40.0,
            90.0,
            25.0,
            50.0,
            0
        );

    TEST_ASSERT_FALSE(result.hasAnomaly);
}

void test_exact_humidity_rate_limit_is_not_anomaly()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            25.0,
            80.0,
            25.0,
            50.0,
            30000
        );

    TEST_ASSERT_FALSE(result.humidityAnomaly);
}

void test_humidity_just_above_rate_limit_is_anomaly()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            25.0,
            80.1,
            25.0,
            50.0,
            30000
        );

    TEST_ASSERT_TRUE(result.humidityAnomaly);
}

void test_rapid_temperature_drop_is_anomaly()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            10.0,
            60.0,
            25.0,
            60.0,
            30000
        );

    TEST_ASSERT_TRUE(result.temperatureAnomaly);
}

void test_rapid_humidity_drop_is_anomaly()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            25.0,
            20.0,
            25.0,
            60.0,
            30000
        );

    TEST_ASSERT_TRUE(result.humidityAnomaly);
}

void test_slow_temperature_change_over_long_interval_is_normal()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            35.0,
            60.0,
            25.0,
            60.0,
            120000
        );

    TEST_ASSERT_FALSE(result.temperatureAnomaly);
}

void test_no_change_is_not_anomaly()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            25.0,
            60.0,
            25.0,
            60.0,
            2000
        );

    TEST_ASSERT_FALSE(result.hasAnomaly);
}

void test_extreme_sensor_change_is_anomaly()
{
    TelemetryAnomalyResult result =
        detectTelemetryAnomaly(
            85.0,
            100.0,
            -40.0,
            0.0,
            2000
        );

    TEST_ASSERT_TRUE(result.hasAnomaly);
    TEST_ASSERT_TRUE(result.temperatureAnomaly);
    TEST_ASSERT_TRUE(result.humidityAnomaly);
}

void setup()
{
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_normal_telemetry_has_no_anomaly);
    RUN_TEST(test_temperature_rate_anomaly_is_detected);
    RUN_TEST(test_humidity_rate_anomaly_is_detected);
    RUN_TEST(test_multiple_anomalies_are_detected);
    RUN_TEST(test_small_changes_are_not_anomalies);
    RUN_TEST(test_exact_temperature_rate_limit_is_not_anomaly);
    RUN_TEST(test_zero_elapsed_time_is_not_processed);
    RUN_TEST(test_exact_humidity_rate_limit_is_not_anomaly);
    RUN_TEST(test_humidity_just_above_rate_limit_is_anomaly);
    RUN_TEST(test_rapid_temperature_drop_is_anomaly);
    RUN_TEST(test_rapid_humidity_drop_is_anomaly);
    RUN_TEST(test_slow_temperature_change_over_long_interval_is_normal);
    RUN_TEST(test_no_change_is_not_anomaly);
    RUN_TEST(test_extreme_sensor_change_is_anomaly);
    
    UNITY_END();
}

void loop()
{
}