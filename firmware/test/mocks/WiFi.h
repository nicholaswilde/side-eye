#pragma once
#include <stdint.h>
#include "Arduino.h"

#define WL_CONNECTED 3
#define WL_IDLE_STATUS 0

class IPAddress {
public:
    String toString() { return "1.2.3.4"; }
};

class WiFiClass {
public:
    uint8_t status() { return WL_CONNECTED; }
    IPAddress softAPIP() { return IPAddress(); }
    IPAddress localIP() { return IPAddress(); }
    int RSSI() { return -50; }
};

extern WiFiClass WiFi;

class WiFiClient {
public:
    bool connected() { return true; }
};

class ESPClass {
public:
    void restart() {}
    uint32_t getFreeHeap() { return 100000; }
};

extern ESPClass ESP;