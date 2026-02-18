#pragma once
#include <string>
#include <functional>

enum t_httpUpdate_return {
    HTTP_UPDATE_FAILED,
    HTTP_UPDATE_NO_UPDATES,
    HTTP_UPDATE_OK
};

class WiFiClientSecure;

class HTTPUpdate {
public:
    void onProgress(std::function<void(int, int)> callback) {}
    t_httpUpdate_return update(WiFiClientSecure& client, std::string url) { return HTTP_UPDATE_OK; }
    int getLastError() { return 0; }
    std::string getLastErrorString() { return "No error"; }
};

extern HTTPUpdate httpUpdate;
