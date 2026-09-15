import sqlite3
from pathlib import Path


DATABASE_PATH = Path(__file__).parent / "telemetry.db"


def get_connection():
    connection = sqlite3.connect(DATABASE_PATH)
    connection.row_factory = sqlite3.Row
    return connection


def initialize_database():
    connection = get_connection()

    connection.execute(
        """
        CREATE TABLE IF NOT EXISTS telemetry (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT NOT NULL,
            firmware_version TEXT NOT NULL,
            uptime_ms INTEGER NOT NULL,
            temperature REAL NOT NULL,
            temperature_quality TEXT,
            humidity REAL NOT NULL,
            humidity_quality TEXT,
            wifi_rssi INTEGER NOT NULL,
            health_state TEXT NOT NULL,
            anomaly_detected INTEGER NOT NULL,
            received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
        """
    )

    columns = connection.execute(
        "PRAGMA table_info(telemetry)"
    ).fetchall()

    column_names = {column["name"] for column in columns}

    if "firmware_version" not in column_names:
        connection.execute(
            """
            ALTER TABLE telemetry
            ADD COLUMN firmware_version TEXT
            """
        )

    if "uptime_ms" not in column_names:
        connection.execute(
            """
            ALTER TABLE telemetry
            ADD COLUMN uptime_ms INTEGER
            """
        )

    if "wifi_rssi" not in column_names:
        connection.execute(
            """
            ALTER TABLE telemetry
            ADD COLUMN wifi_rssi INTEGER
            """
        )

    if "health_state" not in column_names:
        connection.execute(
            """
            ALTER TABLE telemetry
            ADD COLUMN health_state TEXT
            """
        )

    if "anomaly_detected" not in column_names:
        connection.execute(
            """ALTER TABLE telemetry ADD COLUMN anomaly_detected INTEGER"""
        )

    if "temperature_quality" not in column_names:
        connection.execute(
            """
            ALTER TABLE telemetry
            ADD COLUMN temperature_quality TEXT
            """
        )

    if "humidity_quality" not in column_names:
        connection.execute(
            """
            ALTER TABLE telemetry
            ADD COLUMN humidity_quality TEXT
            """
        )

    connection.commit()
    connection.close()


def save_telemetry(
    device_id,
    firmware_version,
    uptime_ms,
    temperature,
    temperature_quality,
    humidity,
    humidity_quality,
    wifi_rssi,
    health_state,
    anomaly_detected,
):
    connection = get_connection()

    try:
        cursor = connection.execute(
            """
            INSERT INTO telemetry (
                device_id,
                firmware_version,
                uptime_ms,
                temperature,
                temperature_quality,
                humidity,
                humidity_quality,
                wifi_rssi,
                health_state,
                anomaly_detected
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                device_id,
                firmware_version,
                uptime_ms,
                temperature,
                temperature_quality,
                humidity,
                humidity_quality,
                wifi_rssi,
                health_state,
                anomaly_detected,
            ),
        )

        connection.commit()

        return cursor.lastrowid

    finally:
        connection.close()


def fetch_telemetry(limit):
    connection = get_connection()

    try:
        rows = connection.execute(
            """
            SELECT
                id,
                device_id,
                firmware_version,
                uptime_ms,
                temperature,
                temperature_quality,
                humidity,
                humidity_quality,
                wifi_rssi,
                health_state,
                anomaly_detected,
                received_at
            FROM telemetry
            ORDER BY id DESC
            LIMIT ?
            """,
            (limit,),
        ).fetchall()

        records = []

        for row in rows:
            record = dict(row)

            if record["anomaly_detected"] is not None:
                record["anomaly_detected"] = bool(record["anomaly_detected"])

            records.append(record)

        return records

    finally:
        connection.close()


def get_health_summary():
    connection = get_connection()

    try:
        rows = connection.execute(
            """
            SELECT
                health_state,
                COUNT(*) AS count
            FROM telemetry
            WHERE health_state IS NOT NULL
            GROUP BY health_state
            """
        ).fetchall()

        summary = {
            "HEALTHY": 0,
            "DEGRADED": 0,
            "FAULT": 0,
        }

        for row in rows:
            summary[row["health_state"]] = row["count"]

        return summary

    finally:
        connection.close()
