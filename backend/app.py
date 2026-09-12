from flask import Flask, jsonify, request
from backend.database import (
    initialize_database,
    save_telemetry,
    fetch_telemetry,
)

app = Flask(__name__)
initialize_database()


@app.get("/health")
def health():
    return jsonify({
        "status": "ok",
        "service": "IoT telemetry backend",
    })


@app.post("/api/telemetry")
def receive_telemetry():
    data = request.get_json(silent=True)

    if not isinstance(data, dict):
        return jsonify({
            "status": "error",
            "message": "Request body must contain a JSON object",
        }), 400

    required_fields = [
        "device_id",
        "firmware_version",
        "uptime_ms",
        "temperature",
        "humidity",
        "wifi_rssi",
        "health_state",
    ]

    missing_fields = [
        field
        for field in required_fields
        if field not in data
    ]

    if missing_fields:
        return jsonify({
            "status": "error",
            "message": "Missing required fields",
            "fields": missing_fields,
        }), 400

    if not isinstance(data["device_id"], str):
        return jsonify({
            "status": "error",
            "message": "device_id must be a string",
        }), 400

    if not isinstance(data["firmware_version"], str):
        return jsonify({
            "status": "error",
            "message": "firmware_version must be a string",
        }), 400

    if type(data["uptime_ms"]) is not int:
        return jsonify({
            "status": "error",
            "message": "uptime_ms must be an integer",
        }), 400

    if type(data["temperature"]) not in (int, float):
        return jsonify({
            "status": "error",
            "message": "temperature must be numeric",
        }), 400

    if type(data["humidity"]) not in (int, float):
        return jsonify({
            "status": "error",
            "message": "humidity must be numeric",
        }), 400

    if type(data["wifi_rssi"]) is not int:
        return jsonify({
            "status": "error",
            "message": "wifi_rssi must be an integer",
        }), 400

    if not isinstance(data["health_state"], str):
        return jsonify({
            "status": "error",
            "message": "health_state must be a string",
        }), 400



    if not data["device_id"].strip():
        return jsonify({
            "status": "error",
            "message": "device_id cannot be empty",
        }), 400

    if not data["firmware_version"].strip():
        return jsonify({
            "status": "error",
            "message": "firmware_version cannot be empty",
        }), 400

    if len(data["device_id"].strip()) > 64:
        return jsonify({
            "status": "error",
            "message": "device_id exceeds maximum length of 64 characters",
        }), 400

    if len(data["firmware_version"].strip()) > 32:
        return jsonify({
            "status": "error",
            "message": "firmware_version exceeds maximum length of 32 characters",
        }), 400

    if data["uptime_ms"] < 0:
        return jsonify({
            "status": "error",
            "message": "uptime_ms cannot be negative",
        }), 400

    if not -40 <= data["temperature"] <= 85:
        return jsonify({
            "status": "error",
            "message": "temperature is outside the supported range",
        }), 400

    if not 0 <= data["humidity"] <= 100:
        return jsonify({
            "status": "error",
            "message": "humidity must be between 0 and 100",
        }), 400

    if not -100 <= data["wifi_rssi"] <= 0:
        return jsonify({
            "status": "error",
            "message": "wifi_rssi is outside the supported range",
        }), 400

    valid_health_states = {
        "HEALTHY",
        "DEGRADED",
        "FAULT",
    }

    if data["health_state"] not in valid_health_states:
        return jsonify({
            "status": "error",
            "message": "health_state must be one of HEALTHY, DEGRADED, FAULT",
        }), 400

    data["device_id"] = data["device_id"].strip()
    data["firmware_version"] = data["firmware_version"].strip()

    record_id = save_telemetry(
        device_id=data["device_id"],
        firmware_version=data["firmware_version"],
        uptime_ms=data["uptime_ms"],
        temperature=data["temperature"],
        humidity=data["humidity"],
        wifi_rssi=data["wifi_rssi"],
        health_state=data["health_state"],
    )

    print("Telemetry received:")
    print(data)

    return jsonify({
        "status": "success",
        "message": "Telemetry received",
        "record_id": record_id,
    }), 201

@app.get("/api/telemetry")
def get_telemetry():
    limit_value = request.args.get("limit")

    if limit_value is None:
        limit = 50
    else:
        try:
            limit = int(limit_value)
        except ValueError:
            return jsonify({
                "status": "error",
                "message": "limit must be an integer",
            }), 400

    if limit < 1:
        return jsonify({
            "status": "error",
            "message": "limit must be greater than 0",
        }), 400

    if limit > 100:
        return jsonify({
            "status": "error",
            "message": "limit cannot exceed 100",
        }), 400

    records = fetch_telemetry(limit)

    return jsonify({
        "status": "success",
        "count": len(records),
        "data": records,
    })


if __name__ == "__main__":
    initialize_database()

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False,
    )
