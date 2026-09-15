#include <Arduino.h>
#include <unity.h>

#include <TelemetryBuffer.h>
#include <TelemetryDelivery.h>
#include <HttpTransport.h>

class FakeHttpTransport : public HttpTransport {
public:
    HttpResult nextResult = HttpResult::SUCCESS;
    String lastPayload;
    uint16_t callCount = 0;

    HttpResult postJson(
        const String& payload
    ) override {
        lastPayload = payload;
        callCount++;

        return nextResult;
    }
};

TelemetryBuffer buffer;
FakeHttpTransport transport;

void setUp() {
    buffer = TelemetryBuffer();
    transport = FakeHttpTransport();
}

void tearDown() {
}

TelemetryDelivery createDelivery() {
    return TelemetryDelivery(
        buffer,
        transport,
        1000,
        10000
    );
}

void test_successful_delivery_removes_payload() {
    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-A")
    );

    TelemetryDelivery delivery =
        createDelivery();

    transport.nextResult =
        HttpResult::SUCCESS;

    TEST_ASSERT_EQUAL(
        DeliveryResult::SUCCESS,
        delivery.process()
    );

    TEST_ASSERT_TRUE(
        buffer.isEmpty()
    );

    TEST_ASSERT_EQUAL_UINT16(
        1,
        transport.callCount
    );

    TEST_ASSERT_EQUAL_STRING(
        "payload-A",
        transport.lastPayload.c_str()
    );
}

void test_transport_failure_retains_payload() {
    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-A")
    );

    TelemetryDelivery delivery =
        createDelivery();

    transport.nextResult =
        HttpResult::TRANSPORT_ERROR;

    TEST_ASSERT_EQUAL(
        DeliveryResult::TRANSPORT_ERROR,
        delivery.process()
    );

    TEST_ASSERT_EQUAL_UINT8(
        1,
        buffer.size()
    );

    String payload;

    TEST_ASSERT_TRUE(
        buffer.peek(payload)
    );

    TEST_ASSERT_EQUAL_STRING(
        "payload-A",
        payload.c_str()
    );

    TEST_ASSERT_EQUAL_UINT16(
        1,
        transport.callCount
    );

    TEST_ASSERT_TRUE(
        delivery.isBackoffActive()
    );

    TEST_ASSERT_EQUAL_UINT8(
        1,
        delivery.failureCount()
    );
}

void test_server_rejection_retains_payload() {
    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-A")
    );

    TelemetryDelivery delivery =
        createDelivery();

    transport.nextResult =
        HttpResult::SERVER_REJECTED;

    TEST_ASSERT_EQUAL(
        DeliveryResult::SERVER_REJECTED,
        delivery.process()
    );

    TEST_ASSERT_EQUAL_UINT8(
        1,
        buffer.size()
    );

    TEST_ASSERT_FALSE(
        delivery.isBackoffActive()
    );

    TEST_ASSERT_EQUAL_UINT8(
        0,
        delivery.failureCount()
    );
}

void test_client_init_failure_retains_payload() {
    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-A")
    );

    TelemetryDelivery delivery =
        createDelivery();

    transport.nextResult =
        HttpResult::CLIENT_INIT_FAILED;

    TEST_ASSERT_EQUAL(
        DeliveryResult::CLIENT_INIT_FAILED,
        delivery.process()
    );

    TEST_ASSERT_EQUAL_UINT8(
        1,
        buffer.size()
    );

    TEST_ASSERT_FALSE(
        delivery.isBackoffActive()
    );
}

void test_fifo_delivery_order() {
    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-A")
    );

    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-B")
    );

    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-C")
    );

    TelemetryDelivery delivery =
        createDelivery();

    transport.nextResult =
        HttpResult::SUCCESS;

    TEST_ASSERT_EQUAL(
        DeliveryResult::SUCCESS,
        delivery.process()
    );

    TEST_ASSERT_EQUAL_STRING(
        "payload-A",
        transport.lastPayload.c_str()
    );

    TEST_ASSERT_EQUAL(
        DeliveryResult::SUCCESS,
        delivery.process()
    );

    TEST_ASSERT_EQUAL_STRING(
        "payload-B",
        transport.lastPayload.c_str()
    );

    TEST_ASSERT_EQUAL(
        DeliveryResult::SUCCESS,
        delivery.process()
    );

    TEST_ASSERT_EQUAL_STRING(
        "payload-C",
        transport.lastPayload.c_str()
    );

    TEST_ASSERT_TRUE(
        buffer.isEmpty()
    );

    TEST_ASSERT_EQUAL_UINT16(
        3,
        transport.callCount
    );
}

void test_successful_delivery_clears_backoff() {
    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-A")
    );

    TelemetryDelivery delivery =
        createDelivery();

    transport.nextResult =
        HttpResult::TRANSPORT_ERROR;

    TEST_ASSERT_EQUAL(
        DeliveryResult::TRANSPORT_ERROR,
        delivery.process()
    );

    TEST_ASSERT_TRUE(
        delivery.isBackoffActive()
    );

    transport.nextResult =
        HttpResult::SUCCESS;

    /*
     * Force the retry window to expire.
     */
    delay(1000);

    TEST_ASSERT_EQUAL(
        DeliveryResult::SUCCESS,
        delivery.process()
    );

    TEST_ASSERT_TRUE(
        buffer.isEmpty()
    );

    TEST_ASSERT_FALSE(
        delivery.isBackoffActive()
    );

    TEST_ASSERT_EQUAL_UINT8(
        0,
        delivery.failureCount()
    );
}

void test_empty_queue_does_not_call_transport() {
    TelemetryDelivery delivery =
        createDelivery();

    TEST_ASSERT_EQUAL(
        DeliveryResult::NO_ACTION,
        delivery.process()
    );

    TEST_ASSERT_EQUAL_UINT16(
        0,
        transport.callCount
    );

    TEST_ASSERT_FALSE(
        delivery.isBackoffActive()
    );
}

void test_exponential_backoff_increases() {
    TEST_ASSERT_TRUE(
        buffer.enqueue("payload-A")
    );

    TelemetryDelivery delivery =
        createDelivery();

    transport.nextResult =
        HttpResult::TRANSPORT_ERROR;

    unsigned long firstAttempt =
        millis();

    TEST_ASSERT_EQUAL(
        DeliveryResult::TRANSPORT_ERROR,
        delivery.process()
    );

    unsigned long firstNext =
        delivery.nextAttemptTime();

    TEST_ASSERT_TRUE(
        firstNext >= firstAttempt + 1000
    );

    /*
     * Wait for the first retry window.
     */
    delay(1000);

    TEST_ASSERT_EQUAL(
        DeliveryResult::TRANSPORT_ERROR,
        delivery.process()
    );

    unsigned long secondNext =
        delivery.nextAttemptTime();

    TEST_ASSERT_TRUE(
        secondNext >= millis() + 1900
    );

    TEST_ASSERT_EQUAL_UINT8(
        2,
        delivery.failureCount()
    );
}

void setup() {
    delay(1000);

    UNITY_BEGIN();

    RUN_TEST(
        test_successful_delivery_removes_payload
    );

    RUN_TEST(
        test_transport_failure_retains_payload
    );

    RUN_TEST(
        test_server_rejection_retains_payload
    );

    RUN_TEST(
        test_client_init_failure_retains_payload
    );

    RUN_TEST(
        test_fifo_delivery_order
    );

    RUN_TEST(
        test_successful_delivery_clears_backoff
    );

    RUN_TEST(
        test_empty_queue_does_not_call_transport
    );

    RUN_TEST(
        test_exponential_backoff_increases
    );

    UNITY_END();
}

void loop() {
}

