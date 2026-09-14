#include <Arduino.h>
#include <unity.h>

#include <device_health.h>

void test_healthy_device()
{
    DeviceHealth health;

    health.sensorHealthy = true;

    TEST_ASSERT_EQUAL(
        DeviceHealthState::HEALTHY,
        evaluateDeviceHealth(
            SystemStatus::RUNNING,
            NetworkStatus::CONNECTED,
            health.sensorHealthy
        )
    );
}

void test_degraded_device_when_network_disconnected()
{
    DeviceHealth health;

    health.sensorHealthy = true;

    TEST_ASSERT_EQUAL(
        DeviceHealthState::DEGRADED,
        evaluateDeviceHealth(
            SystemStatus::RUNNING,
            NetworkStatus::DISCONNECTED,
            health.sensorHealthy
        )
    );
}

void test_fault_when_sensor_unhealthy()
{
    DeviceHealth health;

    health.sensorHealthy = false;

    TEST_ASSERT_EQUAL(
        DeviceHealthState::FAULT,
        evaluateDeviceHealth(
            SystemStatus::RUNNING,
            NetworkStatus::CONNECTED,
            health.sensorHealthy
        )
    );
}

void test_fault_when_system_reports_sensor_error()
{
    DeviceHealth health;

    health.sensorHealthy = false;

    TEST_ASSERT_EQUAL(
        DeviceHealthState::FAULT,
        evaluateDeviceHealth(
            SystemStatus::SENSOR_ERROR,
            NetworkStatus::CONNECTED,
            health.sensorHealthy
        )
    );
}

void test_health_state_names()
{
    TEST_ASSERT_EQUAL_STRING(
        "HEALTHY",
        getDeviceHealthStateName(DeviceHealthState::HEALTHY)
    );

    TEST_ASSERT_EQUAL_STRING(
        "DEGRADED",
        getDeviceHealthStateName(DeviceHealthState::DEGRADED)
    );

    TEST_ASSERT_EQUAL_STRING(
        "FAULT",
        getDeviceHealthStateName(DeviceHealthState::FAULT)
    );
}

void test_health_state_change_detected()
{
    TEST_ASSERT_TRUE(
        hasDeviceHealthStateChanged(
            DeviceHealthState::HEALTHY,
            DeviceHealthState::DEGRADED
        )
    );
}

void test_health_state_change_not_detected()
{
    TEST_ASSERT_FALSE(
        hasDeviceHealthStateChanged(
            DeviceHealthState::HEALTHY,
            DeviceHealthState::HEALTHY
        )
    );
}

void test_degraded_event_counter()
{
    DeviceHealth health;
    resetDeviceHealth(health);

    updateDeviceHealthCounters(
        health,
        DeviceHealthState::HEALTHY,
        DeviceHealthState::DEGRADED
    );

    TEST_ASSERT_EQUAL_UINT32(1, health.degradedEvents);
    TEST_ASSERT_EQUAL_UINT32(0, health.faultEvents);
    TEST_ASSERT_EQUAL_UINT32(0, health.recoveryEvents);
}

void test_fault_event_counter()
{
    DeviceHealth health;
    resetDeviceHealth(health);

    updateDeviceHealthCounters(
        health,
        DeviceHealthState::HEALTHY,
        DeviceHealthState::FAULT
    );

    TEST_ASSERT_EQUAL_UINT32(0, health.degradedEvents);
    TEST_ASSERT_EQUAL_UINT32(1, health.faultEvents);
    TEST_ASSERT_EQUAL_UINT32(0, health.recoveryEvents);
}

void test_recovery_event_counter()
{
    DeviceHealth health;
    resetDeviceHealth(health);

    updateDeviceHealthCounters(
        health,
        DeviceHealthState::DEGRADED,
        DeviceHealthState::HEALTHY
    );

    TEST_ASSERT_EQUAL_UINT32(0, health.degradedEvents);
    TEST_ASSERT_EQUAL_UINT32(0, health.faultEvents);
    TEST_ASSERT_EQUAL_UINT32(1, health.recoveryEvents);
}

void test_healthy_duration() {
    DeviceHealth health;
    resetDeviceHealth(health);

    updateDeviceHealthDuration(
        health,
        DeviceHealthState::HEALTHY,
        5000
    );

    TEST_ASSERT_EQUAL_UINT32(
        5000,
        health.healthyDurationMs
    );
}

void test_degraded_duration() {
    DeviceHealth health;
    resetDeviceHealth(health);

    updateDeviceHealthDuration(
        health,
        DeviceHealthState::DEGRADED,
        3000
    );

    TEST_ASSERT_EQUAL_UINT32(
        3000,
        health.degradedDurationMs
    );
}

void test_fault_duration() {
    DeviceHealth health;
    resetDeviceHealth(health);

    updateDeviceHealthDuration(
        health,
        DeviceHealthState::FAULT,
        2000
    );

    TEST_ASSERT_EQUAL_UINT32(
        2000,
        health.faultDurationMs
    );
}

void test_initial_health_baseline()
{
    DeviceHealth health;
    resetDeviceHealth(health);

    TEST_ASSERT_FALSE(
        health.healthStateInitialized
    );

    DeviceHealthState current =
        evaluateDeviceHealth(
            SystemStatus::BOOTING,
            NetworkStatus::DISCONNECTED,
            true
        );

    updateDeviceHealthState(
        health,
        current,
        1000
    );

    TEST_ASSERT_TRUE(
        health.healthStateInitialized
    );

    TEST_ASSERT_EQUAL(
        DeviceHealthState::DEGRADED,
        health.previousHealthState
    );

    TEST_ASSERT_EQUAL_UINT32(
        0,
        health.degradedEvents
    );

    TEST_ASSERT_EQUAL_UINT32(
        0,
        health.faultEvents
    );

    TEST_ASSERT_EQUAL_UINT32(
        0,
        health.recoveryEvents
    );

    TEST_ASSERT_EQUAL_UINT32(
        1000,
        health.lastHealthEvaluationMs
    );
}

void test_health_state_transition()
{
    DeviceHealth health;
    resetDeviceHealth(health);

    health.healthStateInitialized = true;
    health.previousHealthState =
        DeviceHealthState::DEGRADED;

    DeviceHealthState current =
        DeviceHealthState::HEALTHY;

    TEST_ASSERT_TRUE(
        hasDeviceHealthStateChanged(
            health.previousHealthState,
            current
        )
    );

    updateDeviceHealthCounters(
        health,
        health.previousHealthState,
        current
    );

    TEST_ASSERT_EQUAL_UINT32(
        1,
        health.recoveryEvents
    );

    TEST_ASSERT_EQUAL_UINT32(
        0,
        health.degradedEvents
    );

    TEST_ASSERT_EQUAL_UINT32(
        0,
        health.faultEvents
    );

    health.previousHealthState = current;
}

void test_fault_recovery_transition()
{
    DeviceHealth health;
    resetDeviceHealth(health);

    health.healthStateInitialized = true;
    health.previousHealthState =
        DeviceHealthState::HEALTHY;

    DeviceHealthState current =
        DeviceHealthState::FAULT;

    updateDeviceHealthCounters(
        health,
        health.previousHealthState,
        current
    );

    TEST_ASSERT_EQUAL_UINT32(
        1,
        health.faultEvents
    );

    health.previousHealthState = current;

    current = DeviceHealthState::HEALTHY;

    updateDeviceHealthCounters(
        health,
        health.previousHealthState,
        current
    );

    TEST_ASSERT_EQUAL_UINT32(
        1,
        health.recoveryEvents
    );

    TEST_ASSERT_EQUAL_UINT32(
        1,
        health.faultEvents
    );
}

void test_health_duration_tracking()
{
    DeviceHealth health;
    resetDeviceHealth(health);

    updateDeviceHealthDuration(
        health,
        DeviceHealthState::HEALTHY,
        1000
    );

    updateDeviceHealthDuration(
        health,
        DeviceHealthState::DEGRADED,
        2000
    );

    updateDeviceHealthDuration(
        health,
        DeviceHealthState::FAULT,
        3000
    );

    TEST_ASSERT_EQUAL_UINT32(
        1000,
        health.healthyDurationMs
    );

    TEST_ASSERT_EQUAL_UINT32(
        2000,
        health.degradedDurationMs
    );

    TEST_ASSERT_EQUAL_UINT32(
        3000,
        health.faultDurationMs
    );
}

void setup()
{
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_healthy_device);
    RUN_TEST(test_degraded_device_when_network_disconnected);
    RUN_TEST(test_fault_when_sensor_unhealthy);
    RUN_TEST(test_fault_when_system_reports_sensor_error);
    RUN_TEST(test_health_state_names);
    RUN_TEST(test_health_state_change_detected);
    RUN_TEST(test_health_state_change_not_detected);
    RUN_TEST(test_degraded_event_counter);
    RUN_TEST(test_fault_event_counter);
    RUN_TEST(test_recovery_event_counter);
    RUN_TEST(test_healthy_duration);
    RUN_TEST(test_degraded_duration);
    RUN_TEST(test_fault_duration);
    RUN_TEST(test_initial_health_baseline);
    RUN_TEST(test_health_state_transition);
    RUN_TEST(test_fault_recovery_transition);
    RUN_TEST(test_health_duration_tracking);

   UNITY_END();
}

void loop()
{
}
