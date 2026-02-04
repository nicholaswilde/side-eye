#include <Arduino.h>
#include "DisplayManager.h"
#include "InputHandler.h"
#include "NetworkManager.h"
#include "SyncManager.h"

/* 
 * SideEye Firmware - Orchestrator
 */

#define BTN_PIN 9

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.0.0-unknown"
#endif

SystemState state;
Page currentPage = PAGE_IDENTITY;
unsigned long lastPageChange = 0;
const unsigned long PAGE_DURATION = 5000;
bool needsStaticDraw = true;
bool waitingMessageActive = true;
bool shouldSaveConfig = false;
unsigned long lastMqttRetry = 0;
String deviceID = "";

DisplayManager display;
InputHandler input(BTN_PIN, display);
SideEyeNetworkManager network;
SyncManager syncManager;

String getDeviceID() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char id[7];
    snprintf(id, sizeof(id), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(id);
}

void saveConfigCallback() {
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager) {
    display.drawConfigMode(("SideEye-" + deviceID).c_str(), WiFi.softAPIP().toString());
}

void handleJson(String json) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) return;

    const char* type = doc["type"];
    JsonObject data = doc["data"];

    bool was_connected = state.connected;

    if (strcmp(type, "Identity") == 0) {
        state.hostname = data["hostname"].as<String>();
        state.ip = data["ip"].as<String>();
        state.mac = data["mac"].as<String>();
        state.os = data["os"].as<String>();
        state.user = data["user"].as<String>();
        state.has_data = true;
        state.connected = true;
    } else if (strcmp(type, "Stats") == 0) {
        state.cpu_percent = data["cpu_percent"];
        state.ram_used = data["ram_used"];
        state.ram_total = data["ram_total"];
        state.disk_used = data["disk_used"];
        state.disk_total = data["disk_total"];
        state.net_up = data["net_up"];
        state.net_down = data["net_down"];
        state.uptime = data["uptime"];
        state.has_data = true;
        state.connected = true;
    } else if (strcmp(type, "ListFiles") == 0) {
        String path = data["path"] | "/";
        String list = syncManager.listFiles(path.c_str());
        Serial.print("{\"type\":\"FileList\",\"data\":");
        Serial.print(list);
        Serial.println("}");
    } else if (strcmp(type, "WriteChunk") == 0) {
        state.sd_sync_status = "Syncing...";
        currentPage = PAGE_SD;
        lastPageChange = millis();
        input.notifyActivity();
        needsStaticDraw = true;

        bool success = syncManager.handleWriteChunk(data);
        state.sd_sync_status = success ? "Syncing..." : "Error!";

        Serial.print("{\"type\":\"OperationResult\",\"data\":{\"success\":");
        Serial.print(success ? "true" : "false");
        Serial.println(",\"message\":\"Chunk written\"}}");
    }

    if (state.connected && !was_connected) {
        needsStaticDraw = true;
    }

    if (state.connected) {
        network.publishState(state);
    }

    if (strcmp(type, "GetVersion") == 0) {
        JsonDocument res;
        res["type"] = "Version";
        res["version"] = FIRMWARE_VERSION;
        serializeJson(res, Serial);
        Serial.println();
    }

    display.updateDynamicValues(state, currentPage, needsStaticDraw, waitingMessageActive, FIRMWARE_VERSION);
    needsStaticDraw = false;
}

void setup() {
    Serial.begin(115200);
    deviceID = getDeviceID();
    
    display.begin();
    input.begin();
    syncManager.begin();
    
    display.drawBootScreen(FIRMWARE_VERSION);
    Serial.printf("\n--- SideEye Firmware v%s starting ---\n", FIRMWARE_VERSION);
    delay(2000);

    network.begin(deviceID, FIRMWARE_VERSION, saveConfigCallback, configModeCallback);
    network.saveConfig(shouldSaveConfig);
    
    display.drawWiFiOnline();
    delay(1000);

    display.drawStaticUI(state, currentPage, FIRMWARE_VERSION);
    display.updateDynamicValues(state, currentPage, true, true, FIRMWARE_VERSION);
    waitingMessageActive = false;
    needsStaticDraw = false;
}

String inputBuffer = "";

void loop() {
    if (state.sd_sync_status == "Syncing..." && millis() - lastPageChange > 2000) {
        state.sd_sync_status = "Idle";
        needsStaticDraw = true;
    }

    input.update(state, currentPage, lastPageChange, needsStaticDraw, FIRMWARE_VERSION);
    network.update(lastMqttRetry);

    if (state.has_data) {
        input.notifyActivity();
        state.has_data = false;
    }

    // Timeout for connection status
    static unsigned long lastDataReceived = 0;
    if (state.has_data) lastDataReceived = millis(); // This was missing in logic above

    // Logic for state.connected timeout
    // (Handled by handleJson and has_data flag in original, let's keep it simple)

    // Page cycling
    if (state.connected && input.isScreenOn()) {
        unsigned long now = millis();
        if (now - lastPageChange > PAGE_DURATION) {
            currentPage = static_cast<Page>((currentPage + 1) % NUM_PAGES);
            lastPageChange = now;
            needsStaticDraw = true;
        }
    }

    if (needsStaticDraw) {
        display.updateDynamicValues(state, currentPage, true, false, FIRMWARE_VERSION);
        needsStaticDraw = false;
    }

    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                handleJson(inputBuffer);
            }
            inputBuffer = "";
        } else {
            if (inputBuffer.length() < 512) {
                inputBuffer += c;
            }
        }
    }
}
