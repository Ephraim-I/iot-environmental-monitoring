const API_BASE_URL = "http://localhost:5000";

async function fetchLatestTelemetry(deviceId) {
    const response = await fetch(
        `${API_BASE_URL}/api/telemetry/latest?device_id=${encodeURIComponent(deviceId)}`
    );

    if (!response.ok) {
        throw new Error(
            `Latest telemetry request failed: ${response.status}`
        );
    }

    return response.json();
}


async function fetchHistoricalTelemetry(deviceId, limit = 50) {
    const response = await fetch(
        `${API_BASE_URL}/api/telemetry?device_id=${encodeURIComponent(deviceId)}&limit=${limit}`
    );

    if (!response.ok) {
        throw new Error(
            `Historical telemetry request failed: ${response.status}`
        );
    }

    return response.json();
}

async function fetchDeviceStatus(deviceId) {
    const response = await fetch(
        `${API_BASE_URL}/api/device/status?device_id=${encodeURIComponent(deviceId)}`
    );

    if (!response.ok) {
        throw new Error(
            `Device status request failed: ${response.status}`
        );
    }

    return response.json();
}


async function fetchTelemetryStats() {
    const response = await fetch(
        `${API_BASE_URL}/api/telemetry/stats`
    );

    if (!response.ok) {
        throw new Error(
            `Telemetry statistics request failed: ${response.status}`
        );
    }

    return response.json();
}

