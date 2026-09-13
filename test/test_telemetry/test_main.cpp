#include <Arduino.h>
#include <unity.h>
#include <Telemetry.h>

void test_telemetry_contains_anomaly_false()
{
    TelemetryData data{
        "IOT-ENV-001",
        "1.0.0",
        10000,
        28.5,
        58.0,
        -40,
        "HEALTHY",
        false
    };

    String json = telemetryToJson(data);

    TEST_ASSERT_TRUE(
        json.indexOf("\"anomaly_detected\":false") >= 0
    );
}

void test_telemetry_contains_anomaly_true()
{
    TelemetryData data{
        "IOT-ENV-001",
        "1.0.0",
        10000,
        28.5,
        58.0,
        -40,
        "HEALTHY",
        true
    };

    String json = telemetryToJson(data);

    TEST_ASSERT_TRUE(
        json.indexOf("\"anomaly_detected\":true") >= 0
    );
}

void test_telemetry_contains_health_and_anomaly_separately()
{
    TelemetryData data{
        "IOT-ENV-001",
        "1.0.0",
        10000,
        28.5,
        58.0,
        -40,
        "HEALTHY",
        true
    };

    String json = telemetryToJson(data);

    TEST_ASSERT_TRUE(
        json.indexOf("\"health_state\":\"HEALTHY\"") >= 0
    );

    TEST_ASSERT_TRUE(
        json.indexOf("\"anomaly_detected\":true") >= 0
    );
}

void setup()
{
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_telemetry_contains_anomaly_false);
    RUN_TEST(test_telemetry_contains_anomaly_true);
    RUN_TEST(test_telemetry_contains_health_and_anomaly_separately);

    UNITY_END();
}

void loop() {}
