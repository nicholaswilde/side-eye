#pragma once
#include <functional>
#include <string>

class WebServer {
public:
    WebServer(int port) {}
    void begin() {}
    void handleClient() {}
    void on(const char* uri, std::function<void()> handler) {}
    void send(int code, const char* content_type, std::string content) {}
};
