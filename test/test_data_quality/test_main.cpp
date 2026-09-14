#include <Arduino.h>
#include <unity.h>

#include <DataQuality.h>

void test_data_quality_valid() {
    DataQuality quality = DataQuality::VALID;

    TEST_ASSERT_EQUAL(
        DataQuality::VALID,
        quality
    );
}

void test_data_quality_suspect() {
    DataQuality quality = DataQuality::SUSPECT;

    TEST_ASSERT_EQUAL(
        DataQuality::SUSPECT,
        quality
    );
}

void test_data_quality_invalid() {
    DataQuality quality = DataQuality::INVALID;

    TEST_ASSERT_EQUAL(
        DataQuality::INVALID,
        quality
    );
}

void test_data_quality_states_are_distinct() {
    TEST_ASSERT_NOT_EQUAL(
        DataQuality::VALID,
        DataQuality::SUSPECT
    );

    TEST_ASSERT_NOT_EQUAL(
        DataQuality::VALID,
        DataQuality::INVALID
    );

    TEST_ASSERT_NOT_EQUAL(
        DataQuality::SUSPECT,
        DataQuality::INVALID
    );
}

void setup() {
    UNITY_BEGIN();

    RUN_TEST(test_data_quality_valid);
    RUN_TEST(test_data_quality_suspect);
    RUN_TEST(test_data_quality_invalid);
    RUN_TEST(test_data_quality_states_are_distinct);

    UNITY_END();
}

void loop() {
}
