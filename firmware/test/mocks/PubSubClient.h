#pragma once
#include <Arduino.h>
#include <WiFi.h>

class PubSubClient {
public:
    PubSubClient(WiFiClient& client) {}
    bool connect(const char* id) { return true; }
    bool connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) { return true; }
    bool connect(const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) { return true; }
    void disconnect() {}
    bool publish(const char* topic, const char* payload) { return true; }
    bool publish(const char* topic, const char* payload, bool retained) { return true; }
    bool subscribe(const char* topic) { return true; }
    bool loop() { return true; }
    bool connected() { return true; }
    void setServer(const char* domain, uint16_t port) {}
    int state() { return 0; }
};