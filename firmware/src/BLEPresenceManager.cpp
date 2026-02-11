#include "BLEPresenceManager.h"

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEPresenceManager::BLEPresenceManager() {}

void BLEPresenceManager::begin(const char* deviceId) {
    BLEDevice::init(String("SideEye-" + String(deviceId)).c_str());
    setupServer(deviceId);
    
    _pBLEScan = BLEDevice::getScan();
    _pBLEScan->setAdvertisedDeviceCallbacks(this);
    _pBLEScan->setActiveScan(true);
    _pBLEScan->setInterval(100);
    _pBLEScan->setWindow(99);
}

void BLEPresenceManager::setupServer(const char* deviceId) {
    _pServer = BLEDevice::createServer();
    BLEService* pService = _pServer->createService(SERVICE_UUID);
    _pPresenceCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    pService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
}

void BLEPresenceManager::setEnabled(bool enabled) {
    if (_enabled == enabled) return;
    _enabled = enabled;
    if (_enabled) {
        startScan();
    } else {
        stopScan();
        _present = false;
    }
}

void BLEPresenceManager::startScan() {
    if (_pBLEScan) {
        _pBLEScan->start(0, nullptr, false);
    }
}

void BLEPresenceManager::stopScan() {
    if (_pBLEScan) {
        _pBLEScan->stop();
    }
}

void BLEPresenceManager::onResult(BLEAdvertisedDevice advertisedDevice) {
    _lastSeen = millis();
    if (!_present) {
        _present = true;
        Serial.println("{\"type\": \"Presence\", \"data\": {\"status\": true}}");
    }
}

void BLEPresenceManager::update() {
    if (!_enabled) return;

    if (_present && (millis() - _lastSeen > PRESENCE_TIMEOUT)) {
        _present = false;
        Serial.println("{\"type\": \"Presence\", \"data\": {\"status\": false}}");
    }

    if (_pPresenceCharacteristic) {
        uint8_t val = _present ? 1 : 0;
        _pPresenceCharacteristic->setValue(&val, 1);
        _pPresenceCharacteristic->notify();
    }
}

const char* BLEPresenceManager::getStatusString() const {
    if (!_enabled) return "Disabled";
    return _present ? "Present" : "Scanning";
}
