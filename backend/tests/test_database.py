import sqlite3
from unittest.mock import MagicMock, patch

import pytest

from backend.database import (
    save_telemetry,
    fetch_telemetry,
    fetch_latest_telemetry,
    get_health_summary,
    fetch_telemetry_stats,
)


def make_row(values):
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row

    cursor = connection.execute(
        """
        SELECT
            ? AS id,
            ? AS device_id,
            ? AS firmware_version,
            ? AS uptime_ms,
            ? AS temperature,
            ? AS temperature_quality,
            ? AS humidity,
            ? AS humidity_quality,
            ? AS wifi_rssi,
            ? AS health_state,
            ? AS anomaly_detected
        """,
        values,
    )

    row = cursor.fetchone()
    connection.close()

    return row

def make_stats_row(values):
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row

    cursor = connection.execute(
        """
        SELECT
            ? AS total_records,
            ? AS healthy_records,
            ? AS degraded_records,
            ? AS fault_records,
            ? AS anomalies_detected,
            ? AS valid_temperature,
            ? AS suspect_temperature,
            ? AS invalid_temperature,
            ? AS valid_humidity,
            ? AS suspect_humidity,
            ? AS invalid_humidity
        """,
        values,
    )

    row = cursor.fetchone()
    connection.close()

    return row


def test_fetch_telemetry_returns_records():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchall.return_value = [
        make_row(
            (1, "TEST-001", "1.0.0", 5000, 25.5, "VALID", 60.0, "VALID", -60, "HEALTHY", False)
        ),
        make_row(
            (2, "TEST-002", "1.0.1", 6000, 26.0, "VALID", 55.0, "VALID", -65, "HEALTHY", False)
        ),
    ]

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        records = fetch_telemetry(2)

    assert len(records) == 2
    assert records[0]["device_id"] == "TEST-001"
    assert records[1]["device_id"] == "TEST-002"


def test_fetch_telemetry_respects_limit():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchall.return_value = [
        make_row(
            (1, "TEST-001", "1.0.0", 5000, 25.5, "VALID", 60.0, "VALID", -60, "HEALTHY", False)
        ),
        make_row(
            (2, "TEST-002", "1.0.1", 6000, 26.0, "VALID", 55.0, "VALID", -65, "HEALTHY", False)
        ),
    ]

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        records = fetch_telemetry(2)

    assert len(records) == 2

    mock_connection.execute.assert_called_once()

    sql, parameters = mock_connection.execute.call_args.args

    assert "LIMIT ?" in sql
    assert parameters == (2,)


def test_fetch_telemetry_returns_newest_records_first():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchall.return_value = [
        make_row(
            (3, "TEST-003", "1.0.2", 7000, 27.0, "VALID", 50.0, "VALID", -70, "HEALTHY", False)
        ),
        make_row(
            (2, "TEST-002", "1.0.1", 6000, 26.0, "VALID", 55.0, "VALID", -65, "HEALTHY", False)
        ),
        make_row(
            (1, "TEST-001", "1.0.0", 5000, 25.5, "VALID", 60.0, "VALID", -60, "HEALTHY", False)
        ),
    ]

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        records = fetch_telemetry(3)

    assert len(records) == 3
    assert records[0]["device_id"] == "TEST-003"
    assert records[1]["device_id"] == "TEST-002"
    assert records[2]["device_id"] == "TEST-001"


def test_fetch_telemetry_filters_by_device():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchall.return_value = [
        make_row(
            (
                10,
                "IOT-ENV-001",
                "1.0.0",
                5000,
                28.5,
                "VALID",
                70.0,
                "VALID",
                -52,
                "HEALTHY",
                False,
            )
        ),
        make_row(
            (
                11,
                "IOT-ENV-001",
                "1.0.0",
                6000,
                28.8,
                "VALID",
                71.0,
                "VALID",
                -51,
                "HEALTHY",
                False,
            )
        ),
    ]

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        records = fetch_telemetry(
            2,
            device_id="IOT-ENV-001",
        )

    assert len(records) == 2
    assert all(
        record["device_id"] == "IOT-ENV-001"
        for record in records
    )

    mock_connection.execute.assert_called_once()

    query = mock_connection.execute.call_args.args[0]
    parameters = mock_connection.execute.call_args.args[1]

    assert "WHERE device_id = ?" in query
    assert "LIMIT ?" in query
    assert parameters == ("IOT-ENV-001", 2)


def test_save_telemetry_closes_connection_on_success():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.lastrowid = 123
    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        record_id = save_telemetry(
            "TEST-001",
            "1.0.0",
            5000,
            25.5,
            "VALID",
            60.0,
            "VALID",
            -60,
            "HEALTHY",
            False,
        )

    assert record_id == 123
    mock_connection.commit.assert_called_once()
    mock_connection.close.assert_called_once()


def test_save_telemetry_closes_connection_on_database_error():
    mock_connection = MagicMock()

    mock_connection.execute.side_effect = sqlite3.Error(
        "simulated database failure"
    )

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        with pytest.raises(sqlite3.Error):
            save_telemetry(
                "TEST-001",
                "1.0.0",
                5000,
                25.5,
                "VALID",
                60.0,
                "VALID",
                -60,
                "HEALTHY",
                False,
            )

    mock_connection.close.assert_called_once()


def test_get_health_summary_returns_health_state_counts():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchall.return_value = [
        make_row(
            (1, "TEST-001", "1.0.0", 5000, 25.5, "VALID", 60.0, "VALID", -60, "HEALTHY", False)
        ),
        make_row(
            (2, "TEST-002", "1.0.0", 6000, 26.0, "VALID", 55.0, "VALID", -65, "HEALTHY", False)
        ),
        make_row(
            (3, "TEST-003", "1.0.0", 7000, 27.0, "VALID", 50.0, "VALID", -70, "DEGRADED", False)
        ),
        make_row(
            (4, "TEST-004", "1.0.0", 8000, 28.0, "VALID", 45.0, "VALID", -75, "FAULT", False)
        ),
    ]

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        summary = get_health_summary()

    assert summary == {
        "HEALTHY": 2,
        "DEGRADED": 1,
        "FAULT": 1,
    }


def make_health_summary_row(health_state, count):
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row

    cursor = connection.execute(
        """
        SELECT
            ? AS health_state,
            ? AS count
        """,
        (health_state, count),
    )

    row = cursor.fetchone()
    connection.close()

    return row

def test_get_health_summary_returns_health_state_counts():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchall.return_value = [
        make_health_summary_row("HEALTHY", 2),
        make_health_summary_row("DEGRADED", 1),
        make_health_summary_row("FAULT", 1),
    ]

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        summary = get_health_summary()

    assert summary == {
        "HEALTHY": 2,
        "DEGRADED": 1,
        "FAULT": 1,
    }


def test_get_health_summary_returns_zero_counts_when_empty():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchall.return_value = []

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        summary = get_health_summary()

    assert summary == {
        "HEALTHY": 0,
        "DEGRADED": 0,
        "FAULT": 0,
    }

def test_fetch_latest_telemetry_returns_newest_record():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchone.return_value = make_row(
        (
            5,
            "TEST-005",
            "1.0.0",
            9000,
            29.0,
            "VALID",
            65.0,
            "VALID",
            -55,
            "HEALTHY",
            False,
        )
    )

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        record = fetch_latest_telemetry()

    assert record["id"] == 5
    assert record["device_id"] == "TEST-005"
    assert record["temperature"] == 29.0
    assert record["humidity"] == 65.0
    assert record["anomaly_detected"] is False


def test_fetch_latest_telemetry_returns_newest_record_for_device():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchone.return_value = make_row(
        (
            10,
            "IOT-ENV-001",
            "1.0.0",
            39449,
            28.8,
            "VALID",
            71.0,
            "VALID",
            -51,
            "HEALTHY",
            False,
        )
    )

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        record = fetch_latest_telemetry("IOT-ENV-001")

    assert record["id"] == 10
    assert record["device_id"] == "IOT-ENV-001"
    assert record["temperature"] == 28.8
    assert record["humidity"] == 71.0
    assert record["health_state"] == "HEALTHY"
    assert record["anomaly_detected"] is False

    mock_connection.execute.assert_called_once()

    query = mock_connection.execute.call_args.args[0]
    parameters = mock_connection.execute.call_args.args[1]

    assert "WHERE device_id = ?" in query
    assert parameters == ("IOT-ENV-001",)


def test_fetch_latest_telemetry_returns_none_when_empty():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchone.return_value = None
    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        record = fetch_latest_telemetry()

    assert record is None


def test_fetch_telemetry_stats_returns_expected_counts():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchone.return_value = make_stats_row(
        (
            3,  # total_records
            1,  # healthy_records
            1,  # degraded_records
            1,  # fault_records
            2,  # anomalies_detected
            1,  # valid_temperature
            1,  # suspect_temperature
            1,  # invalid_temperature
            2,  # valid_humidity
            0,  # suspect_humidity
            1,  # invalid_humidity
        )
    )

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        stats = fetch_telemetry_stats()

    assert stats["total_records"] == 3

    assert stats["healthy_records"] == 1
    assert stats["degraded_records"] == 1
    assert stats["fault_records"] == 1

    assert stats["anomalies_detected"] == 2

    assert stats["valid_temperature"] == 1
    assert stats["suspect_temperature"] == 1
    assert stats["invalid_temperature"] == 1

    assert stats["valid_humidity"] == 2
    assert stats["suspect_humidity"] == 0
    assert stats["invalid_humidity"] == 1

    mock_connection.close.assert_called_once()


def test_fetch_telemetry_stats_returns_zero_counts_when_empty():
    mock_connection = MagicMock()
    mock_cursor = MagicMock()

    mock_cursor.fetchone.return_value = make_stats_row(
        (
            0,  # total_records
            0,  # healthy_records
            0,  # degraded_records
            0,  # fault_records
            0,  # anomalies_detected
            0,  # valid_temperature
            0,  # suspect_temperature
            0,  # invalid_temperature
            0,  # valid_humidity
            0,  # suspect_humidity
            0,  # invalid_humidity
        )
    )

    mock_connection.execute.return_value = mock_cursor

    with patch(
        "backend.database.get_connection",
        return_value=mock_connection,
    ):
        stats = fetch_telemetry_stats()

    assert stats["total_records"] == 0

    assert stats["healthy_records"] == 0
    assert stats["degraded_records"] == 0
    assert stats["fault_records"] == 0

    assert stats["anomalies_detected"] == 0

    assert stats["valid_temperature"] == 0
    assert stats["suspect_temperature"] == 0
    assert stats["invalid_temperature"] == 0

    assert stats["valid_humidity"] == 0
    assert stats["suspect_humidity"] == 0
    assert stats["invalid_humidity"] == 0

    mock_connection.close.assert_called_once()
