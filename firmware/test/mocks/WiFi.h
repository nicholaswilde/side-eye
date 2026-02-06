#pragma once
class WiFiClass {
public:
    int status() { return 3; } // WL_CONNECTED
    String macAddress() { return "00:00:00:00:00:00"; }
    int RSSI() { return -50; }
};
extern WiFiClass WiFi;

#define WL_CONNECTED 3
