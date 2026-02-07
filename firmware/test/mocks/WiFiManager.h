#pragma once
#include <Arduino.h>
#include <functional>

enum wm_debuglevel_t {
    WM_DEBUG_OFF = 0,
    WM_DEBUG_ERROR = 1,
    WM_DEBUG_NOTIFY = 2,
    WM_DEBUG_VERBOSE = 3,
    WM_DEBUG_DEV = 4
};

class WiFiManagerParameter {
public:
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length) {}
    const char* getValue() { return ""; }
};

class WiFiManager {
public:
    void setSaveConfigCallback(void (*func)()) {}
    void setAPCallback(void (*func)(WiFiManager*)) {}
    void setWebServerCallback(void (*func)()) {}
    void addParameter(WiFiManagerParameter *p) {}
    bool autoConnect(const char *apName, const char *apPassword = NULL) { return true; }
    void resetSettings() {}
    bool preloadWiFi(String ssid, String pass) { return true; }
};
