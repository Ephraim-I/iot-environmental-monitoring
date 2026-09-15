#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>
#include <HttpTransport.h>

class HttpClient : public HttpTransport {
public:
    explicit HttpClient(const char* endpoint);

    HttpResult postJson(
        const String& payload
    ) override;

private:
    const char* _endpoint;
};

#endif
