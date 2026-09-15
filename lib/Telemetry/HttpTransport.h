#ifndef HTTP_TRANSPORT_H
#define HTTP_TRANSPORT_H

#include <Arduino.h>

enum class HttpResult {
    SUCCESS,
    CLIENT_INIT_FAILED,
    TRANSPORT_ERROR,
    SERVER_REJECTED
};

class HttpTransport {
public:
    virtual ~HttpTransport() = default;

    virtual HttpResult postJson(
        const String& payload
    ) = 0;
};

#endif

