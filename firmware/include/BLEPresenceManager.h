#ifndef BLE_PRESENCE_MANAGER_H
#define BLE_PRESENCE_MANAGER_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEServer.h>
#include <ArduinoJson.h>

class BLEPresenceManager : public BLEAdvertisedDeviceCallbacks {
public:
    BLEPresenceManager();
    void begin(const char* deviceId);
    void update();
    void setEnabled(bool enabled);
    bool isEnabled() const { return _enabled; }
    bool isPresent() const { return _present; }
    const char* getStatusString() const;

    // Callbacks
    void onResult(BLEAdvertisedDevice advertisedDevice) override;

private:
    bool _enabled = false;
    bool _present = false;
    bool _lastSentPresence = false;
    unsigned long _lastSeen = 0;
    const unsigned long PRESENCE_TIMEOUT = 10000; // 10 seconds
    String _targetMac = "";
    
    BLEScan* _pBLEScan = nullptr;
    BLEServer* _pServer = nullptr;
    BLECharacteristic* _pPresenceCharacteristic = nullptr;

    void startScan();
    void stopScan();
    void setupServer(const char* deviceId);
};

#endif
