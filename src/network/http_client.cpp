#include "http_client.h"

#include <HTTPClient.h>

HttpClient::HttpClient(const char* endpoint)
    : _endpoint(endpoint) {
}

bool HttpClient::postJson(const String& payload) {
    HTTPClient http;

    http.setConnectTimeout(5000);
    http.setTimeout(5000);

    if (!http.begin(_endpoint)) {
        Serial.println("HTTP client initialization failed.");
        return false;
    }

    http.addHeader("Content-Type", "application/json");

    int responseCode = http.POST(payload);

    if (responseCode > 0) {
        Serial.print("HTTP response code: ");
        Serial.println(responseCode);

        String response = http.getString();

        Serial.print("Server response: ");
        Serial.println(response);

        http.end();

        if (responseCode >= 200 && responseCode < 300) {
            return true;
        }

        Serial.println("HTTP request returned an unsuccessful status code.");

        return false;
    }

    Serial.print("HTTP request failed: ");
    Serial.println(http.errorToString(responseCode));

    http.end();

    return false;
}
