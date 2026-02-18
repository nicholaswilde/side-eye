#pragma once
#include <string>
#include "Arduino.h"

class BLEAddress {
public:
    BLEAddress(const char* addr) : _addr(addr) {}
    String toString() { return String(_addr.c_str()); }
private:
    std::string _addr;
};

class BLEAdvertisedDevice {
public:
    BLEAddress getAddress() { return BLEAddress("AA:BB:CC:DD:EE:FF"); }
    int getRSSI() { return -70; }
};

class BLEAdvertisedDeviceCallbacks {
public:
    virtual ~BLEAdvertisedDeviceCallbacks() {}
    virtual void onResult(BLEAdvertisedDevice advertisedDevice) = 0;
};

class BLEScan {
public:
    void setAdvertisedDeviceCallbacks(BLEAdvertisedDeviceCallbacks* pCallbacks) {}
    void setActiveScan(bool active) {}
    void setInterval(uint16_t interval) {}
    void setWindow(uint16_t window) {}
    void start(uint32_t duration, void (*scanCompleteCB)(void*), bool is_continue = false) {}
    void stop() {}
};

class BLECharacteristic {
public:
    static const uint32_t PROPERTY_READ = 1;
    static const uint32_t PROPERTY_NOTIFY = 2;
    void setValue(uint8_t* data, size_t size) {}
    void notify() {}
};

class BLEService {
public:
    BLECharacteristic* createCharacteristic(const char* uuid, uint32_t properties) { return new BLECharacteristic(); }
    void start() {}
};

class BLEAdvertising {
public:
    void addServiceUUID(const char* uuid) {}
    void setScanResponse(bool set) {}
    void start() {}
};

class BLEServer {
public:
    BLEService* createService(const char* uuid) { return new BLEService(); }
};

class BLEDevice {
public:
    static void init(const char* deviceName) {}
    static BLEScan* getScan() { 
        static BLEScan* scan = new BLEScan();
        return scan; 
    }
    static BLEServer* createServer() { return new BLEServer(); }
    static BLEAdvertising* getAdvertising() { 
        static BLEAdvertising* adv = new BLEAdvertising();
        return adv; 
    }
};

class BLEUtils {
public:
};
