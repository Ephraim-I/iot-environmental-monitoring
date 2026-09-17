const DEVICE_ID = "IOT-ENV-001";
const DASHBOARD_REFRESH_INTERVAL_MS = 10000;
const DEVICE_STALE_THRESHOLD_MS = 30000;

let dashboardRefreshInProgress = false;
let dashboardInitialized = false;

function setConnectionStatus(message) {
    const element = document.getElementById(
        "connection-status"
    );

    if (!element) {
        return;
    }

    element.textContent = message;
}


function updateElement(id, value) {
    if (typeof id !== "string" || id.length === 0) {
        return;
    }

    const element = document.getElementById(id);

    if (element) {
        element.textContent = value;
    }
}


function getQualityClass(quality) {
    if (typeof quality !== "string") {
        return "";
    }

    switch (quality) {
        case "VALID":
            return "status-valid";

        case "SUSPECT":
            return "status-suspect";

        case "INVALID":
            return "status-invalid";

        default:
            return "";
    }
}


function getHealthClass(health) {
    if (typeof health !== "string") {
        return "";
    }

    switch (health) {
        case "HEALTHY":
            return "status-healthy";

        case "DEGRADED":
            return "status-degraded";

        case "FAULT":
            return "status-fault";

        default:
            return "";
    }
}


function setStatus(elementId, text, className) {
    if (
        typeof elementId !== "string" ||
        elementId.length === 0
    ) {
        return;
    }

    const element = document.getElementById(elementId);

    if (!element) {
        return;
    }

    const safeClassName =
        typeof className === "string"
            ? className
            : "";

    element.textContent = text;
    element.className =
        `status-badge ${safeClassName}`.trim();
}


function displayLatestTelemetry(response) {
    if (
        !response ||
        typeof response !== "object"
    ) {
        throw new Error(
            "Telemetry statistics response is invalid"
        );
    }

    if (
        !response ||
        typeof response !== "object" ||
        !response.data ||
        typeof response.data !== "object"
    ) {
        throw new Error(
            "Latest telemetry response contains invalid data"
        );
    }

    const telemetry = response.data;

    updateElement(
        "temperature-value",
        `${telemetry.temperature} °C`
    );

    updateElement(
        "humidity-value",
        `${telemetry.humidity} %RH`
    );

    updateElement(
        "rssi-value",
        `${telemetry.wifi_rssi} dBm`
    );

    updateElement(
        "health-value",
        telemetry.health_state
    );

    setStatus(
        "temperature-quality",
        telemetry.temperature_quality,
        getQualityClass(telemetry.temperature_quality)
    );

    setStatus(
        "humidity-quality",
        telemetry.humidity_quality,
        getQualityClass(telemetry.humidity_quality)
    );

    setStatus(
        "health-status",
        telemetry.health_state,
        getHealthClass(telemetry.health_state)
    );

    updateElement(
        "network-quality",
        "CONNECTED"
    );

    updateElement(
        "device-id",
        telemetry.device_id
    );

    updateElement(
        "firmware-version",
        telemetry.firmware_version
    );

    updateElement(
        "uptime",
        `${telemetry.uptime_ms} ms`
    );

    updateElement(
        "last-telemetry",
        telemetry.received_at
    );

    updateElement(
        "anomaly-status",
        telemetry.anomaly_detected
            ? "DETECTED"
            : "NONE"
    );
}


function displayTelemetryStats(response) {
    const stats = response.data;

    if (
        !stats ||
        typeof stats !== "object"
    ) {
        throw new Error(
            "Telemetry statistics response contains invalid data"
        );
    }

    updateElement(
        "total-records",
        stats.total_records
    );

    updateElement(
        "anomalies",
        stats.anomalies_detected
    );

    updateElement(
        "healthy-records",
        stats.healthy_records
    );

    updateElement(
        "degraded-records",
        stats.degraded_records
    );

    updateElement(
        "fault-records",
        stats.fault_records
    );

    updateElement(
        "valid-temperature",
        stats.valid_temperature
    );

    updateElement(
        "suspect-temperature",
        stats.suspect_temperature
    );

    updateElement(
        "invalid-temperature",
        stats.invalid_temperature
    );

    updateElement(
        "valid-humidity",
        stats.valid_humidity
    );

    updateElement(
        "suspect-humidity",
        stats.suspect_humidity
    );

    updateElement(
        "invalid-humidity",
        stats.invalid_humidity
    );
}

function formatChartTime(timestamp) {
    const date = new Date(timestamp);

    if (Number.isNaN(date.getTime())) {
        return "Unknown time";
    }

    return date.toLocaleString(
        [],
        {
            month: "short",
            day: "numeric",
            hour: "2-digit",
            minute: "2-digit"
        }
    );
}

function renderTelemetryChart(
    chartId,
    records,
    valueKey,
    unit
) {
    const svg = document.getElementById(chartId);

    if (!svg) {
        return;
    }

    svg.innerHTML = "";

    const qualityKey =
        valueKey === "temperature"
            ? "temperature_quality"
            : "humidity_quality";

const validRecords = records
    .filter(record => {
        if (
            record[qualityKey] !== "VALID" ||
            typeof record[valueKey] !== "number" ||
            !Number.isFinite(record[valueKey]) ||
            typeof record.received_at !== "string"
        ) {
            return false;
        }

        const timestamp = new Date(
            record.received_at.replace(" ", "T")
        ).getTime();

        return Number.isFinite(timestamp);
    })
    .slice()
    .reverse();

if (validRecords.length === 0) {
    const message = document.createElementNS(
        "http://www.w3.org/2000/svg",
        "text"
    );

    message.setAttribute(
        "x",
        "400"
    );

    message.setAttribute(
        "y",
        "150"
    );

    message.setAttribute(
        "text-anchor",
        "middle"
    );

    message.setAttribute(
        "class",
        "chart-label"
    );

    message.textContent =
        records.length === 0
            ? "No telemetry data available"
            : "No valid telemetry measurements available";

    svg.appendChild(message);

    return;
}

    const width = 800;
    const height = 300;

    const padding = {
        top: 20,
        right: 20,
        bottom: 40,
        left: 50
    };

    const chartWidth =
        width - padding.left - padding.right;

    const chartHeight =
        height - padding.top - padding.bottom;

    const values = validRecords.map(
        record => record[valueKey]
    );

let minValue = Math.min(...values);
let maxValue = Math.max(...values);

if (minValue === maxValue) {
    minValue -= 1;
    maxValue += 1;
}

const valueRange =
    maxValue - minValue;

    const timestamps = validRecords.map(
        record =>
            new Date(
                record.received_at.replace(" ", "T")
            ).getTime()
    );

    const minTime = Math.min(...timestamps);
    const maxTime = Math.max(...timestamps);

    const timeRange =
        maxTime - minTime || 1;

    const points = validRecords.map(
        (record, index) => {
            const timestamp =
                timestamps[index];

            const x =
                padding.left +
                (
                    (timestamp - minTime) /
                    timeRange
                ) *
                chartWidth;

            const y =
                padding.top +
                (
                    1 -
                    (
                        record[valueKey] - minValue
                    ) /
                    valueRange
                ) *
                chartHeight;

            return `${x},${y}`;
        }
    );

    const line = document.createElementNS(
        "http://www.w3.org/2000/svg",
        "polyline"
    );

    line.setAttribute(
        "points",
        points.join(" ")
    );

    line.setAttribute(
        "fill",
        "none"
    );

    line.setAttribute(
        "stroke",
        "currentColor"
    );

    line.setAttribute(
        "stroke-width",
        "2"
    );

    svg.appendChild(line);

    const minLabel =
        document.createElementNS(
            "http://www.w3.org/2000/svg",
            "text"
        );

    minLabel.setAttribute(
        "x",
        "5"
    );

    minLabel.setAttribute(
        "class",
        "chart-label"
    );

    minLabel.setAttribute(
        "y",
        String(height - padding.bottom)
    );

    minLabel.textContent =
        `${minValue.toFixed(1)} ${unit}`;

    svg.appendChild(minLabel);

    const maxLabel =
        document.createElementNS(
            "http://www.w3.org/2000/svg",
            "text"
        );

    maxLabel.setAttribute(
        "x",
        "5"
    );

    maxLabel.setAttribute(
        "class",
        "chart-label"
    );

    maxLabel.setAttribute(
        "y",
        String(padding.top + 5)
    );

    maxLabel.textContent =
        `${maxValue.toFixed(1)} ${unit}`;

    svg.appendChild(maxLabel);

    const firstTimeLabel =
        document.createElementNS(
            "http://www.w3.org/2000/svg",
            "text"
        );

    firstTimeLabel.setAttribute(
        "x",
        String(padding.left)
    );

    firstTimeLabel.setAttribute(
        "class",
        "chart-label"
    );

    firstTimeLabel.setAttribute(
        "y",
        String(height - 10)
    );

    firstTimeLabel.setAttribute(
        "text-anchor",
        "start"
    );

    firstTimeLabel.textContent =
        formatChartTime(minTime);

    svg.appendChild(firstTimeLabel);


    const lastTimeLabel =
        document.createElementNS(
            "http://www.w3.org/2000/svg",
            "text"
        );

    lastTimeLabel.setAttribute(
        "x",
        String(width - padding.right)
    );

    lastTimeLabel.setAttribute(
        "class",
        "chart-label"
    );

    lastTimeLabel.setAttribute(
        "y",
        String(height - 10)
    );

    lastTimeLabel.setAttribute(
        "text-anchor",
        "end"
    );

    lastTimeLabel.textContent =
        formatChartTime(maxTime);

    svg.appendChild(lastTimeLabel);
}

async function loadHistoricalTelemetry(limit) {
    const historicalTelemetry =
        await fetchHistoricalTelemetry(
            DEVICE_ID,
            limit
        );

    if (
        !historicalTelemetry ||
        typeof historicalTelemetry !== "object" ||
        !Array.isArray(historicalTelemetry.data)
    ) {
        throw new Error(
            "Historical telemetry response contains invalid data"
        );
    }

    const records = historicalTelemetry.data;

    renderTelemetryChart(
        "temperature-chart",
        records,
        "temperature",
        "°C"
    );

    renderTelemetryChart(
        "humidity-chart",
        records,
        "humidity",
        "%RH"
    );

    console.log(
        `Historical telemetry (${limit} records):`,
        records
    );
}

function setDeviceConnectionStatus(status) {
    const element = document.getElementById(
        "device-connection-status"
    );

    if (!element) {
        return;
    }

    element.textContent = status;
}

function updateDeviceConnectionStatus(deviceStatus) {
    if (!deviceStatus) {
        setDeviceConnectionStatus("Device: Unknown");
        return;
    }

    if (!deviceStatus.received_at) {
        setDeviceConnectionStatus("Device: Unknown");
        return;
    }

    const receivedAt = new Date(
        deviceStatus.received_at.replace(" ", "T")
    );

    if (Number.isNaN(receivedAt.getTime())) {
        setDeviceConnectionStatus("Device: Unknown");
        return;
    }

    const ageMs = Math.max(
        0,
        Date.now() - receivedAt.getTime()
    );

    const ageText = formatDataAge(ageMs);

    if (ageMs <= DEVICE_STALE_THRESHOLD_MS) {
        setDeviceConnectionStatus(
            `Device: Online (${ageText})`
        );
    } else {
        setDeviceConnectionStatus(
            `Device: Stale (${ageText})`
        );
    }
}

function formatDataAge(ageMs) {
    const ageSeconds = Math.floor(ageMs / 1000);

    if (ageSeconds < 60) {
        return `${ageSeconds}s ago`;
    }

    const ageMinutes = Math.floor(ageSeconds / 60);

    if (ageMinutes < 60) {
        return `${ageMinutes}m ago`;
    }

    const ageHours = Math.floor(ageMinutes / 60);

    return `${ageHours}h ago`;
}

function getSelectedHistoryLimit() {
    const element = document.getElementById(
        "history-limit"
    );

    if (!element) {
        return 50;
    }

    const limit = Number(element.value);

    if (!Number.isFinite(limit) || limit < 1) {
        return 50;
    }

    return limit;
}

async function loadDashboard() {
    if (dashboardRefreshInProgress) {
        return;
    }

    dashboardRefreshInProgress = true;

    try {
        if (!dashboardInitialized) {
            setConnectionStatus("API: Connecting...");
        }

        const results = await Promise.allSettled([
            fetchLatestTelemetry(DEVICE_ID),
            fetchDeviceStatus(DEVICE_ID),
            fetchTelemetryStats()
        ]);

        const [
            latestTelemetryResult,
            deviceStatusResult,
            telemetryStatsResult
        ] = results;

        if (
            latestTelemetryResult.status === "fulfilled"
        ) {
            displayLatestTelemetry(
                latestTelemetryResult.value
            );
        } else {
            console.error(
                "Latest telemetry request failed:",
                latestTelemetryResult.reason
            );
        }

        if (
            deviceStatusResult.status === "fulfilled"
        ) {
            const deviceStatusResponse =
                deviceStatusResult.value;

            if (
                !deviceStatusResponse ||
                typeof deviceStatusResponse !== "object"
            ) {
                throw new Error(
                    "Device status response is invalid"
                );
            }

            updateDeviceConnectionStatus(
                deviceStatusResponse.data
            );

            console.log(
                "Device status:",
                deviceStatusResponse.data
            );
        } else {
            console.error(
                "Device status request failed:",
                deviceStatusResult.reason
            );

            setDeviceConnectionStatus(
                "Device: Unknown"
            );
        }

        if (
            telemetryStatsResult.status === "fulfilled"
        ) {
            displayTelemetryStats(
                telemetryStatsResult.value
            );
        } else {
            console.error(
                "Telemetry statistics request failed:",
                telemetryStatsResult.reason
            );
        }

        const historyLimit = getSelectedHistoryLimit();

        try {
            await loadHistoricalTelemetry(historyLimit);
        } catch (error) {
            console.error(
                "Historical telemetry request failed:",
                error
            );
        }

        const apiRequestFailed =
            latestTelemetryResult.status === "rejected" &&
            deviceStatusResult.status === "rejected" &&
            telemetryStatsResult.status === "rejected";

        if (apiRequestFailed) {
            setConnectionStatus("API: Error");
        } else {
            setConnectionStatus("API: Connected");
            dashboardInitialized = true;
        }

    } catch (error) {
        console.error(
            "Dashboard loading failed:",
            error
        );

        setConnectionStatus("API: Error");

    } finally {
        dashboardRefreshInProgress = false;
    }
}

loadDashboard();

const historyLimitElement = document.getElementById(
    "history-limit"
);

if (historyLimitElement) {
    historyLimitElement.addEventListener(
        "change",
        async () => {
            const historyLimit = getSelectedHistoryLimit();

            try {
                await loadHistoricalTelemetry(historyLimit);
            } catch (error) {
                console.error(
                    "Historical telemetry refresh failed:",
                    error
                );
            }
        }
    );
}

const refreshDashboardElement = document.getElementById(
    "refresh-dashboard"
);

if (refreshDashboardElement) {
    refreshDashboardElement.addEventListener(
        "click",
        async () => {
            refreshDashboardElement.disabled = true;
            refreshDashboardElement.textContent = "Refreshing...";

            try {
                await loadDashboard();
            } finally {
                refreshDashboardElement.disabled = false;
                refreshDashboardElement.textContent = "Refresh";
            }
        }
    );
}

setInterval(
    loadDashboard,
    DASHBOARD_REFRESH_INTERVAL_MS
);
