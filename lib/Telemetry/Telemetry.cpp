#include "Telemetry.h"
#include <DataQuality.h>

const char* dataQualityToString(
    DataQuality quality
) {
    switch (quality) {
        case DataQuality::VALID:
            return "VALID";

        case DataQuality::SUSPECT:
            return "SUSPECT";

        case DataQuality::INVALID:
            return "INVALID";

        default:
            return "INVALID";
    }
}

String telemetryToJson(const TelemetryData& data) {
    String json = "{";

    json += "\"device_id\":\"";
    json += data.deviceId;
    json += "\",";

    json += "\"firmware_version\":\"";
    json += data.firmwareVersion;
    json += "\",";

    json += "\"uptime_ms\":";
    json += String(data.uptimeMs);
    json += ",";

    json += "\"temperature\":";
    json += String(data.temperature, 1);
    json += ",";
    
    json += "\"temperature_quality\":\"";
    json += data.temperatureQuality;
    json += "\",";
    
    json += "\"humidity\":";
    json += String(data.humidity, 1);
    json += ",";
    
    json += "\"humidity_quality\":\"";
    json += data.humidityQuality;
    json += "\",";

    json += "\"wifi_rssi\":";
    json += String(data.wifiRssi);
    json += ",";

    json += "\"health_state\":\"";
    json += data.healthState;
    json += "\"";

    json += ",\"anomaly_detected\":";
    json += data.anomalyDetected ? "true" : "false";

    json += "}";

    return json;
}
