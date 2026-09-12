import pytest

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
            "humidity": 60.0,
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
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
            "humidity": 60.0,
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
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

def test_telemetry_rejects_missing_field(client):
    response = client.post(
        "/api/telemetry",
        json={
            "device_id": "TEST-001",
            "firmware_version": "1.0.0",
            "uptime_ms": 5000,
            "temperature": 25.5,
            "humidity": 60.0,
            "health_state": "HEALTHY",
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
            "humidity": 60.0,
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
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
            "humidity": 150.0,
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
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
                "humidity": 60.0,
                "wifi_rssi": -60,
                "health_state": "HEALTHY",
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
            "humidity": 60.0,
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
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
            "humidity": 60.0,
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
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
            "humidity": 60.0,
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
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
            "humidity": 60.0,
            "wifi_rssi": -60,
            "health_state": "HEALTHY",
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
                "humidity": 60.0,
                "wifi_rssi": -60,
                "health_state": "HEALTHY",
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
                "humidity": 60.0,
                "wifi_rssi": -60,
                "health_state": "HEALTHY",
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

    