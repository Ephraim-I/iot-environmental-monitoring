#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>

enum class HttpResult {
    SUCCESS,
    CLIENT_INIT_FAILED,
    TRANSPORT_ERROR,
    SERVER_REJECTED
};

class HttpClient {
public:
    HttpClient(const char* endpoint);

    HttpResult postJson(const String& payload);

private:
    const char* _endpoint;
};

#endif