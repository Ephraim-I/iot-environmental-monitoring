#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>

class HttpClient {
public:
    HttpClient(const char* endpoint);

    bool postJson(const String& payload);

private:
    const char* _endpoint;
};

#endif