#include <Arduino.h>
#include <unity.h>

#include <SamplingConfig.h>

#include "../../src/config/sampling_config.h"

void test_sampling_config_stores_interval() {
    SamplingConfig config{
        2000
    };

    TEST_ASSERT_EQUAL_UINT32(
        2000,
        config.intervalMs
    );
}

void test_sampling_config_allows_different_interval() {
    SamplingConfig config{
        5000
    };

    TEST_ASSERT_EQUAL_UINT32(
        5000,
        config.intervalMs
    );
}

void test_sampling_config_accepts_application_default() {
    SamplingConfig config{
        DEFAULT_SAMPLING_INTERVAL_MS
    };

    TEST_ASSERT_EQUAL_UINT32(
        2000,
        config.intervalMs
    );
}

void setup() {
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(
        test_sampling_config_stores_interval
    );

    RUN_TEST(
        test_sampling_config_allows_different_interval
    );

    RUN_TEST(
        test_sampling_config_accepts_application_default
    );

    UNITY_END();
}

void loop() {
}
