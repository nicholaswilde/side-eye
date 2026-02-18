#pragma once
#include <string>
#include "WiFiClientSecure.h"

#define HTTP_CODE_OK 200

class HTTPClient {
public:
    bool begin(WiFiClientSecure& client, std::string url) { return true; }
    void setUserAgent(const char* ua) {}
    int GET() { return HTTP_CODE_OK; }
    std::string getString() { return "{\"tag_name\":\"v9.9.9\"}"; }
    void end() {}
};
