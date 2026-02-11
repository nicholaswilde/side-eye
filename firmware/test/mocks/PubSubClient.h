#pragma once
#include <Arduino.h>
#include <WiFi.h>

class PubSubClient {
public:
    PubSubClient(WiFiClient& client) : _connected(false), _state(0) {}
    bool connect(const char* id) { _connected = true; return true; }
    bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) { _connected = true; return true; }
    bool connect(const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) { _connected = true; return true; }
    void disconnect() { _connected = false; }
    bool publish(const char* topic, const char* payload) { return true; }
    bool publish(const char* topic, const char* payload, bool retained) { return true; }
    bool subscribe(const char* topic) { return true; }
    bool loop() { return true; }
    bool connected() { return _connected; }
    void setServer(const char* domain, uint16_t port) {}
    void setCallback(std::function<void(char*, uint8_t*, unsigned int)> callback) {}
    int state() { return _state; }
    
    void _setConnected(bool c) { _connected = c; }
    void _setState(int s) { _state = s; }

private:
    bool _connected;
    int _state;
};
