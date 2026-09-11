#include "http_client.h"

#include "device/logger.h"

#include <HTTPClient.h>

HttpClient::HttpClient(const char* endpoint)
    : _endpoint(endpoint) {
}

HttpResult HttpClient::postJson(const String& payload) {
    HTTPClient http;

    http.setConnectTimeout(5000);
    http.setTimeout(5000);

    if (!http.begin(_endpoint)) {
        Logger::error(
            "HTTP",
            "HTTP client initialization failed"
        );

        return HttpResult::CLIENT_INIT_FAILED;
    }

    http.addHeader("Content-Type", "application/json");

    int responseCode = http.POST(payload);

    if (responseCode > 0) {
        Logger::infof(
            "HTTP",
            "Response code: %d",
            responseCode
        );

        String response = http.getString();

        Logger::infof(
            "HTTP",
            "Server response: %s",
            response.c_str()
        );

        http.end();

        if (responseCode >= 200 && responseCode < 300) {
            return HttpResult::SUCCESS;
        }

        Logger::warningf(
            "HTTP",
            "Request returned unsuccessful status code: %d",
            responseCode
        );

        return HttpResult::SERVER_REJECTED;
    }

    Logger::errorf(
        "HTTP",
        "Request failed: %s",
        http.errorToString(responseCode).c_str()
    );

    http.end();

    return HttpResult::TRANSPORT_ERROR;
}
