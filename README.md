# IoT Environmental Monitoring & Telemetry System

An ESP32-based environmental monitoring and telemetry system built as an engineering lab and portfolio project.

The project goes beyond basic sensor logging. It demonstrates how an embedded device can collect measurements, validate data, evaluate device health, detect telemetry anomalies, buffer data during network failures, deliver telemetry reliably, persist historical records, and expose the resulting data through a web dashboard.

> **Architecture:** DHT11 → ESP32 → HTTP/JSON → Flask API → SQLite → Web Dashboard

![Environmental Monitoring Dashboard](docs/images/dashboard-overview.png)

## Table of Contents

- [Project Overview](#project-overview)
- [Why This Project](#why-this-project)
- [System Architecture](#system-architecture)
- [Core Features](#core-features)
- [Telemetry Model](#telemetry-model)
- [Reliable Delivery](#reliable-delivery)
- [Hardware](#hardware)
- [Firmware Architecture](#firmware-architecture)
- [Backend](#backend)
- [Database](#database)
- [Web Dashboard](#web-dashboard)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Configuration](#configuration)
- [API Reference](#api-reference)
- [Testing](#testing)
- [Engineering Design Decisions](#engineering-design-decisions)
- [Known Limitations](#known-limitations)
- [Future Development](#future-development)
- [License](#license)

## Project Overview

The system uses an **ESP32** and a **DHT11** sensor to periodically collect temperature and humidity measurements.

The firmware is organized into independent modules for:

- Sensor abstraction
- Sampling
- Measurement representation
- Measurement validation
- Data-quality classification
- Device-health evaluation
- Telemetry generation
- Telemetry anomaly detection
- Telemetry buffering
- Reliable telemetry delivery
- Wi-Fi management
- HTTP transport
- Device status
- Operational metrics
- Logging
- Configuration

Collected telemetry is sent over HTTP as JSON to a **Flask** backend. The backend validates and stores the data in **SQLite**, while a browser-based dashboard retrieves the data through the API and presents current state, historical trends, and system statistics.

## Why This Project

This project is designed around an important IoT engineering principle:

> **A useful IoT system must remain understandable and predictable when things go wrong.**

Instead of treating the ESP32 as a device that simply reads a sensor and sends a value, the system explicitly models:

1. Whether a measurement is trustworthy.
2. Whether the device itself appears healthy.
3. Whether telemetry looks anomalous.
4. What happens when the network disappears.
5. What happens when the backend is unavailable.
6. How buffered telemetry is eventually delivered.
7. How historical telemetry can be inspected after transmission.

This makes the project useful as a foundation for larger embedded, IoT, monitoring, and edge-computing systems.

## System Architecture

```text
┌──────────────────────┐
│        DHT11         │
│ Temperature/Humidity │
└──────────┬───────────┘
           │
           ▼
┌────────────────────────────────┐
│              ESP32             │
│                                │
│ Sensor → Sampling → Validation │
│             │                  │
│       Data Quality             │
│       Device Health            │
│       Anomaly Detection        │
│             │                  │
│         Telemetry              │
│             │                  │
│      FIFO Buffer               │
│             │                  │
│     Reliable Delivery          │
└────────────┬───────────────────┘
             │
          HTTP/JSON
             │
             ▼
┌──────────────────────────────┐
│          Flask API           │
│                              │
│ Telemetry Ingestion          │
│ Validation                   │
│ History / Latest Data        │
│ Device Status                │
│ Statistics / Health Summary  │
└────────────┬─────────────────┘
             │
             ▼
┌──────────────────────────────┐
│            SQLite            │
│     Historical Telemetry     │
└────────────┬─────────────────┘
             │
             ▼
┌──────────────────────────────┐
│        Web Dashboard         │
│                              │
│ Current State                │
│ Device Status                │
│ Historical Trends            │
│ Data Quality                 │
│ Health & Anomaly Statistics  │
└──────────────────────────────┘
```

### Separation of Responsibilities

| Layer | Responsibility |
|---|---|
| Sensor | Acquire physical measurements |
| Sampling | Determine when measurements are collected |
| Validation | Check measurements against configured ranges |
| Data Quality | Classify measurement trustworthiness |
| Device Health | Represent device/sensing-pipeline condition |
| Anomaly Detection | Identify suspicious telemetry behavior |
| Telemetry | Package measurements and metadata |
| Buffer | Retain telemetry awaiting delivery |
| Delivery | Attempt reliable transmission |
| HTTP Transport | Handle concrete network requests |
| Flask API | Receive, validate, and expose telemetry |
| SQLite | Persist historical records |
| Dashboard | Visualize system state and history |

## Core Features

### Embedded Telemetry Collection

- ESP32-based environmental monitoring
- DHT11 temperature and humidity sensing
- Configurable sampling interval
- Structured JSON telemetry
- Device identification
- Firmware-version reporting
- Wi-Fi RSSI monitoring

### Measurement Validation

Measurements are evaluated against configured operating ranges before being accepted into the telemetry pipeline.

Current DHT11 configuration:

| Measurement | Range | Unit |
|---|---:|---|
| Temperature | 0–50 | °C |
| Humidity | 20–90 | %RH |

### Data-Quality Classification

Measurements can be classified as:

- `VALID`
- `SUSPECT`
- `INVALID`

Temperature and humidity quality are tracked independently.

### Device-Health Monitoring

Device health is represented as:

- `HEALTHY`
- `DEGRADED`
- `FAULT`

### Telemetry Anomaly Detection

Telemetry anomaly state is represented independently:

- `NONE`
- `DETECTED`

This produces three separate dimensions of system state:

```text
DEVICE HEALTH
HEALTHY / DEGRADED / FAULT

DATA QUALITY
VALID / SUSPECT / INVALID

ANOMALY
NONE / DETECTED
```

For example, a device can be `HEALTHY` while producing a `SUSPECT` measurement.

## Reliable Telemetry Delivery

Telemetry is placed into a fixed-size FIFO buffer before delivery.

The core delivery invariant is:

```text
PEEK
  │
  ▼
Attempt HTTP delivery
  │
  ├── SUCCESS ──► DEQUEUE
  │
  └── FAILURE ──► RETAIN
```

The current telemetry buffer has a capacity of **5 records**.

When the backend is unavailable:

- Existing queued telemetry is retained.
- The oldest record remains at the front of the queue.
- Records are delivered in FIFO order.
- New records are dropped when the buffer is full.
- Existing records are not overwritten.
- Transport failures trigger exponential backoff.
- Delivery resumes when the backend becomes available again.

## Hardware

- ESP32 development board
- DHT11 temperature/humidity sensor
- Breadboard
- Jumper wires
- USB power/data connection

## Firmware Architecture

```text
Sensor
  │
  ▼
Sampler
  │
  ▼
Measurement
  │
  ├── Validation
  ├── Data Quality
  └── Anomaly Detection
  │
  ▼
Telemetry
  │
  ▼
Telemetry Buffer
  │
  ▼
Telemetry Delivery
  │
  ▼
HTTP Transport
```

Supporting infrastructure includes:

```text
Wi-Fi Manager
Logger
System Status
Device Status
Telemetry Metrics
Configuration
```

The HTTP transport layer is abstracted through `HttpTransport`, allowing delivery behavior to be tested independently from the concrete HTTP implementation.

## Telemetry Pipeline

A typical sampling cycle follows this sequence:

1. The sampler determines whether a measurement is due.
2. The DHT11 produces temperature and humidity readings.
3. Measurements are validated against configured ranges.
4. Data quality is classified.
5. Telemetry anomaly detection is performed.
6. Device health is evaluated.
7. A structured telemetry payload is created.
8. The payload is added to the telemetry buffer.
9. The delivery component attempts transmission when network connectivity is available.
10. The Flask API validates and stores the telemetry.

## Backend

The backend is implemented with:

- Python
- Flask
- Flask-CORS
- SQLite
- Pytest

Dependencies are defined in:

```text
backend/requirements.txt
```

The API provides:

- Telemetry ingestion
- Historical telemetry retrieval
- Latest telemetry retrieval
- Device status
- Health summaries
- Telemetry statistics
- Device-specific telemetry filtering

## Database

SQLite provides local persistence for historical telemetry.

Stored telemetry includes fields such as:

- Record ID
- Device ID
- Firmware version
- Device uptime
- Temperature
- Temperature quality
- Humidity
- Humidity quality
- Wi-Fi RSSI
- Device health
- Anomaly state
- Server receive timestamp

## Web Dashboard

The dashboard is a static frontend built with:

- HTML
- CSS
- JavaScript
- SVG-based charts

The frontend communicates only with the Flask API.

### Current State

- Temperature
- Humidity
- Wi-Fi RSSI
- Device health
- Temperature quality
- Humidity quality
- Network quality
- Anomaly status

### Device Information

- Device ID
- Firmware version
- Uptime
- Last telemetry timestamp

### Historical Trends

- Temperature history
- Humidity history
- Configurable history size
- Refresh control

Historical charts only plot valid numeric measurements with valid timestamps.

## Getting Started

### Prerequisites

- ESP32 development board
- DHT11 sensor
- PlatformIO
- Python 3
- Git
- Chromium or another modern web browser

### Build Firmware

```bash
git clone <repository-url>
cd IoT_Engineering_Lab

pio run
```

Run firmware tests:

```bash
pio test
```

Upload:

```bash
pio run --target upload
```

Monitor serial output:

```bash
pio device monitor
```

Serial monitor speed:

```text
115200 baud
```

### Start Backend

```bash
cd backend

python3 -m venv .venv
source .venv/bin/activate

pip install -r requirements.txt

python app.py
```

The API listens on:

```text
http://localhost:5000
```

### Start Dashboard

In another terminal:

```bash
cd dashboard
python3 -m http.server 8000
```

Open:

```text
http://localhost:8000
```

## Configuration

Firmware configuration is located under:

```text
src/config/
```

Important values include:

- Device ID
- Device name
- Firmware version
- Sampling interval
- Sensor pins
- Wi-Fi settings
- Telemetry endpoint

### Network Credentials

Local credentials should be supplied through:

```text
src/config/network_config.local.h
```

The safe template is:

```text
src/config/network_config.h
```

Example:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define TELEMETRY_ENDPOINT "http://YOUR_SERVER_IP:5000/api/telemetry"
```

**Never commit real Wi-Fi passwords, API keys, tokens, or other private credentials to Git.**

## API Reference

### Health Check

```http
GET /health
```

### Submit Telemetry

```http
POST /api/telemetry
```

### Retrieve Telemetry

```http
GET /api/telemetry
```

Optional parameters:

```text
limit
device_id
```

### Retrieve Latest Telemetry

```http
GET /api/telemetry/latest
```

Optional parameter:

```text
device_id
```

### Retrieve Health Summary

```http
GET /api/telemetry/health
```

### Retrieve Device Status

```http
GET /api/device/status?device_id=IOT-ENV-001
```

### Retrieve Telemetry Statistics

```http
GET /api/telemetry/stats
```

## Testing

The project contains separate automated test suites for firmware modules and backend behavior.

### Firmware

Current reported result:

```text
115 test cases
115 succeeded
```

Run:

```bash
pio test
```

### Backend

Current reported result:

```text
43 passed
```

Run:

```bash
cd backend
pytest
```

## Engineering Design Decisions

### Modular Firmware

Functionality is separated into focused components instead of being concentrated in `main.cpp`.

### Explicit Data Quality

A sensor reading is not automatically treated as trustworthy simply because the sensor produced a number.

### Separate Health and Measurement Quality

A questionable measurement does not automatically imply device failure.

### FIFO Telemetry Buffering

The queue preserves ordering and avoids silently replacing older unsent telemetry with newer data.

### Peek-Before-Dequeue

```text
peek → send → success → dequeue
```

Failed requests therefore do not accidentally discard telemetry.

### Transport Abstraction

`HttpTransport` separates delivery logic from the concrete HTTP client.

### API as the Dashboard Boundary

The dashboard accesses data through the Flask API rather than directly accessing SQLite.

## Project Structure

```text
IoT_Engineering_Lab/
├── backend/
│   ├── app.py
│   ├── database.py
│   ├── requirements.txt
│   └── tests/
│
├── dashboard/
│   ├── index.html
│   ├── css/
│   └── js/
│
├── docs/
│   └── images/
│
├── include/
│
├── lib/
│   ├── DHT11Sensor/
│   ├── DeviceHealth/
│   ├── DeviceStatus/
│   ├── Measurement/
│   ├── MeasurementValidation/
│   ├── Sampling/
│   ├── Sensor/
│   ├── Telemetry/
│   ├── TelemetryAnomaly/
│   ├── TelemetryBuffer/
│   ├── TelemetryDelivery/
│   └── TelemetryMetrics/
│
├── src/
│   ├── config/
│   ├── device/
│   ├── network/
│   └── main.cpp
│
├── test/
│
├── platformio.ini
└── README.md
```

## Known Limitations

- DHT11 is a basic environmental sensor.
- Telemetry buffering is currently in RAM, so queued records are lost if the ESP32 restarts.
- SQLite is intended for local development and small workloads.
- HTTP is currently used for telemetry transport.
- The system currently targets a single environmental-monitoring node.
- Authentication and authorization are not implemented.
- HTTPS/TLS deployment is not currently configured.
- Alerting and remote device management are not yet implemented.

## Future Development

### Connectivity

- MQTT transport
- HTTPS/TLS
- Persistent telemetry queues
- More robust network recovery

### Device Management

- Multiple sensor nodes
- Device registration
- Provisioning
- Remote configuration
- Fleet monitoring

### Sensing

- Additional environmental sensors
- CO₂ monitoring
- Soil/environmental sensors
- More precise sensors

### Data & Intelligence

- Advanced anomaly detection
- Historical data export
- Long-term trend analysis
- Predictive maintenance experiments

### Backend & Infrastructure

- PostgreSQL
- Authentication and authorization
- Docker deployment
- Production observability
- Automated CI testing

### User Experience

- Alert notifications
- Configurable thresholds
- Historical analytics
- Role-based dashboards

## Project Status

The current implementation demonstrates a complete local IoT telemetry loop:

```text
Physical sensing
      ↓
Embedded processing
      ↓
Validation + quality + health
      ↓
Telemetry buffering + reliable delivery
      ↓
HTTP API
      ↓
Persistent storage
      ↓
Historical visualization
```

The architecture is intentionally modular so that individual components can be replaced or extended without redesigning the entire system.

## License

This project is currently intended as an engineering portfolio and learning project.
