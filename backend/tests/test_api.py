import pytest

from unittest.mock import patch

from backend.app import app
from backend.database import initialize_database


@pytest.fixture
def client(tmp_path, monkeypatch):
    database_path = tmp_path / "test_telemetry.db"

    monkeypatch.setattr(
        "backend.database.DATABASE_PATH",
        database_path,
    )

    initialize_database()

    app.config["TESTING"] = True

    with app.test_client() as test_client:
        yield test_client


def test_health_endpoint(client):
    response = client.get("/health")

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "ok"


def test_telemetry_submission(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    assert response.status_code == 201

    data = response.get_json()

    assert data["status"] == "success"
    assert "record_id" in data


def test_telemetry_retrieval(client):
    client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    response = client.get("/api/telemetry")

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"
    assert data["count"] == 1
    assert len(data["data"]) == 1

    record = data["data"][0]

    assert record["device_id"] == "TEST-001"
    assert record["firmware_version"] == "1.0.0"
    assert record["uptime_ms"] == 5000
    assert record["temperature"] == 25.5
    assert record["humidity"] == 60.0
    assert record["wifi_rssi"] == -60
    assert record["health_state"] == "HEALTHY"
    assert record["anomaly_detected"] is False


def test_telemetry_retrieval_filters_by_device(client):
    telemetry_records = [
        {
            "device_id": "IOT-ENV-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 28.5,
            "temperature_quality": "VALID",
            "humidity": 70.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -52,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
        {
            "device_id": "IOT-ENV-002",
            "firmware_version": "1.0.0",
            "uptime_ms": 6000,
            "temperature": 30.0,
            "temperature_quality": "VALID",
            "humidity": 65.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
        {
            "device_id": "IOT-ENV-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 7000,
            "temperature": 29.0,
            "temperature_quality": "VALID",
            "humidity": 72.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -50,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    ]

    for telemetry in telemetry_records:
        response = client.post(
            "/api/telemetry",
            json=telemetry,
        )

        assert response.status_code == 201

    response = client.get(
        "/api/telemetry?device_id=IOT-ENV-001"
    )

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"
    assert data["count"] == 2
    assert len(data["data"]) == 2

    assert all(
        record["device_id"] == "IOT-ENV-001"
        for record in data["data"]
    )

def test_telemetry_rejects_missing_field(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert "wifi_rssi" in data["fields"]

def test_telemetry_rejects_invalid_type(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": "not-an-integer",
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "uptime_ms must be an integer"


def test_telemetry_rejects_invalid_range(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 150.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "humidity must be between 0 and 100"

def test_telemetry_rejects_missing_json(client):
    response = client.post(
        "/api/telemetry",
        data="not-json",
        content_type="text/plain",
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "Request body must contain a JSON object"


def test_telemetry_rejects_json_array(client):
    response = client.post(
        "/api/telemetry",
        json=[
            {
                "device_id": "TEST-001",
                "firmware_version": "1.0.0",
                "uptime_ms": 5000,
                "temperature": 25.5,
                "temperature_quality": "VALID",
                "humidity": 60.0,
                "humidity_quality": "VALID",
                "wifi_rssi": -60,
                "health_state": "HEALTHY",
                "anomaly_detected": False,
            }
        ],
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "Request body must contain a JSON object"

def test_telemetry_rejects_boolean_uptime(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": True,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "uptime_ms must be an integer"


def test_telemetry_rejects_device_id_too_long(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "A" * 65,
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == (
        "device_id exceeds maximum length of 64 characters"
    )


def test_telemetry_rejects_firmware_version_too_long(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1" * 33,
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == (
        "firmware_version exceeds maximum length of 32 characters"
    )


def test_telemetry_normalizes_string_fields(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "  TEST-NORMALIZE-001  ",
            "firmware_version": "  1.2.3  ",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    assert response.status_code == 201

    data = response.get_json()

    assert data["status"] == "success"

    record_id = data["record_id"]

    response = client.get("/api/telemetry")

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"

    records = data["data"]

    record = next(
        record for record in records
        if record["id"] == record_id
    )

    assert record["device_id"] == "TEST-NORMALIZE-001"
    assert record["firmware_version"] == "1.2.3"

def test_telemetry_retrieval_respects_limit(client):
    for index in range(5):
        response = client.post(
            "/api/telemetry",
            json={
                "device_id": f"TEST-{index:03d}",
                "firmware_version": "1.0.0",
                "uptime_ms": 5000,
                "temperature": 25.5,
                "temperature_quality": "VALID",
                "humidity": 60.0,
                "humidity_quality": "VALID",
                "wifi_rssi": -60,
                "health_state": "HEALTHY",
                "anomaly_detected": False,
            },
        )

        assert response.status_code == 201

    response = client.get("/api/telemetry?limit=2")

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"
    assert data["count"] == 2
    assert len(data["data"]) == 2


def test_telemetry_retrieval_rejects_zero_limit(client):
    response = client.get("/api/telemetry?limit=0")

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "limit must be greater than 0"

def test_telemetry_retrieval_rejects_excessive_limit(client):
    response = client.get("/api/telemetry?limit=101")

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "limit cannot exceed 100"

def test_telemetry_retrieval_uses_default_limit(client):
    for index in range(55):
        response = client.post(
            "/api/telemetry",
            json={
                "device_id": f"TEST-{index:03d}",
                "firmware_version": "1.0.0",
                "uptime_ms": 5000,
                "temperature": 25.5,
                "temperature_quality": "VALID",
                "humidity": 60.0,
                "humidity_quality": "VALID",
                "wifi_rssi": -60,
                "health_state": "HEALTHY",
                "anomaly_detected": False,
            },
        )

        assert response.status_code == 201

    response = client.get("/api/telemetry")

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"
    assert data["count"] == 50
    assert len(data["data"]) == 50

def test_telemetry_retrieval_rejects_non_integer_limit(client):
    response = client.get("/api/telemetry?limit=abc")

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "limit must be an integer"


def test_health_summary_returns_health_state_counts(client):
    with patch(
        "backend.app.get_health_summary",
        return_value={
            "HEALTHY": 12,
            "DEGRADED": 3,
            "FAULT": 1,
        },
    ):
        response = client.get("/api/telemetry/health")

    assert response.status_code == 200

    body = response.get_json()

    assert body["status"] == "success"
    assert body["data"] == {
        "HEALTHY": 12,
        "DEGRADED": 3,
        "FAULT": 1,
    }


def test_health_summary_returns_zero_counts_when_empty(client):
    with patch(
        "backend.app.get_health_summary",
        return_value={
            "HEALTHY": 0,
            "DEGRADED": 0,
            "FAULT": 0,
        },
    ):
        response = client.get("/api/telemetry/health")

    assert response.status_code == 200

    body = response.get_json()

    assert body["status"] == "success"
    assert body["data"] == {
        "HEALTHY": 0,
        "DEGRADED": 0,
        "FAULT": 0,
    }


def test_health_summary_aggregates_real_telemetry(client):
    telemetry_records = [
        {
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
        {
            "device_id": "TEST-002",
            "firmware_version": "1.0.0",
            "uptime_ms": 6000,
            "temperature": 26.0,
            "temperature_quality": "VALID",
            "humidity": 55.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -65,
            "health_state": "DEGRADED",
            "anomaly_detected": False,
        },
        {
            "device_id": "TEST-003",
            "firmware_version": "1.0.0",
            "uptime_ms": 7000,
            "temperature": 27.0,
            "temperature_quality": "VALID",
            "humidity": 50.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -70,
            "health_state": "FAULT",
            "anomaly_detected": False,
        },
        {
            "device_id": "TEST-004",
            "firmware_version": "1.0.0",
            "uptime_ms": 8000,
            "temperature": 28.0,
            "temperature_quality": "VALID",
            "humidity": 45.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -75,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    ]

    for telemetry in telemetry_records:
        response = client.post(
            "/api/telemetry",
            json=telemetry,
        )

        assert response.status_code == 201

    response = client.get("/api/telemetry/health")

    assert response.status_code == 200

    body = response.get_json()

    assert body["status"] == "success"
    assert body["data"] == {
        "HEALTHY": 2,
        "DEGRADED": 1,
        "FAULT": 1,
    }

def test_telemetry_rejects_invalid_anomaly_type(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": "false",
        },
    )

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "anomaly_detected must be a boolean"

def test_telemetry_persists_anomaly_state(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-ANOMALY-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 45.0,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": True,
        },
    )

    assert response.status_code == 201

    record_id = response.get_json()["record_id"]

    response = client.get("/api/telemetry")

    assert response.status_code == 200

    records = response.get_json()["data"]

    record = next(
        record for record in records
        if record["id"] == record_id
    )

    assert record["health_state"] == "HEALTHY"
    assert record["anomaly_detected"] is True


def test_latest_telemetry_endpoint(client):
    client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    response = client.get("/api/telemetry/latest")

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"
    assert data["data"]["device_id"] == "TEST-001"
    assert data["data"]["temperature"] == 25.5
    assert data["data"]["humidity"] == 60.0

def test_latest_telemetry_endpoint_returns_404_when_empty(client):
    response = client.get("/api/telemetry/latest")

    assert response.status_code == 404

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "No telemetry data available"


def test_device_status_endpoint(client):
    client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "temperature_quality": "VALID",
            "humidity": 60.0,
            "humidity_quality": "VALID",
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
            "anomaly_detected": False,
        },
    )

    response = client.get(
        "/api/device/status?device_id=TEST-001"
    )

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"
    assert data["data"]["device_id"] == "TEST-001"
    assert data["data"]["firmware_version"] == "1.0.0"
    assert data["data"]["wifi_rssi"] == -60
    assert data["data"]["health_state"] == "HEALTHY"

def test_device_status_requires_device_id(client):
    response = client.get("/api/device/status")

    assert response.status_code == 400

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == "device_id is required"

def test_device_status_returns_404_for_unknown_device(client):
    response = client.get(
        "/api/device/status?device_id=UNKNOWN-001"
    )

    assert response.status_code == 404

    data = response.get_json()

    assert data["status"] == "error"
    assert data["message"] == (
        "No telemetry data found for device"
    )

def test_telemetry_stats_endpoint(client):
    with patch(
        "backend.app.fetch_telemetry_stats",
        return_value={
            "total_records": 3,
            "healthy_records": 1,
            "degraded_records": 1,
            "fault_records": 1,
            "anomalies_detected": 2,
            "valid_temperature": 1,
            "suspect_temperature": 1,
            "invalid_temperature": 1,
            "valid_humidity": 2,
            "suspect_humidity": 0,
            "invalid_humidity": 1,
        },
    ):
        response = client.get("/api/telemetry/stats")

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"
    assert data["data"]["total_records"] == 3
    assert data["data"]["healthy_records"] == 1
    assert data["data"]["degraded_records"] == 1
    assert data["data"]["fault_records"] == 1
    assert data["data"]["anomalies_detected"] == 2


def test_telemetry_stats_endpoint_returns_zero_counts_when_empty(client):
    with patch(
        "backend.app.fetch_telemetry_stats",
        return_value={
            "total_records": 0,
            "healthy_records": 0,
            "degraded_records": 0,
            "fault_records": 0,
            "anomalies_detected": 0,
            "valid_temperature": 0,
            "suspect_temperature": 0,
            "invalid_temperature": 0,
            "valid_humidity": 0,
            "suspect_humidity": 0,
            "invalid_humidity": 0,
        },
    ):
        response = client.get("/api/telemetry/stats")

    assert response.status_code == 200

    data = response.get_json()

    assert data["status"] == "success"
    assert data["data"]["total_records"] == 0
