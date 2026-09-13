import sqlite3
from unittest.mock import MagicMock, patch

import pytest

from backend.database import (
    save_telemetry,
    fetch_telemetry,
    get_health_summary,
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
            ? AS humidity,
            ? AS wifi_rssi,
            ? AS health_state,
            ? AS anomaly_detected
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
            (1, "TEST-001", "1.0.0", 5000, 25.5, 60.0, -60, "HEALTHY", False)
        ),
        make_row(
            (2, "TEST-002", "1.0.1", 6000, 26.0, 55.0, -65, "HEALTHY", False)
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
            (1, "TEST-001", "1.0.0", 5000, 25.5, 60.0, -60, "HEALTHY", False)
        ),
        make_row(
            (2, "TEST-002", "1.0.1", 6000, 26.0, 55.0, -65, "HEALTHY", False)
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
            (3, "TEST-003", "1.0.2", 7000, 27.0, 50.0, -70, "HEALTHY", False)
        ),
        make_row(
            (2, "TEST-002", "1.0.1", 6000, 26.0, 55.0, -65, "HEALTHY", False)
        ),
        make_row(
            (1, "TEST-001", "1.0.0", 5000, 25.5, 60.0, -60, "HEALTHY", False)
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
            60.0,
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
                60.0,
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
            (1, "TEST-001", "1.0.0", 5000, 25.5, 60.0, -60, "HEALTHY", False)
        ),
        make_row(
            (2, "TEST-002", "1.0.0", 6000, 26.0, 55.0, -65, "HEALTHY", False)
        ),
        make_row(
            (3, "TEST-003", "1.0.0", 7000, 27.0, 50.0, -70, "DEGRADED", False)
        ),
        make_row(
            (4, "TEST-004", "1.0.0", 8000, 28.0, 45.0, -75, "FAULT", False)
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