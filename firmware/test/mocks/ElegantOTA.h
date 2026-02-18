#pragma once
#include "WebServer.h"

class ElegantOTAClass {
public:
    void begin(WebServer* server) {}
    void loop() {}
    void onStart(std::function<void()> callback) {}
    void onProgress(std::function<void(size_t, size_t)> callback) {}
    void onEnd(std::function<void(bool)> callback) {}
};

extern ElegantOTAClass ElegantOTA;
