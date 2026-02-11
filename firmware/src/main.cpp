#include <Arduino.h>
#include "DisplayManager.h"
#include "InputHandler.h"
#include "NetworkManager.h"
#include "SyncManager.h"
#include "BLEPresenceManager.h"

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
BLEPresenceManager blePresence;

void onMqttMessage(char* topic, uint8_t* payload, unsigned int length) {
    String topicStr = String(topic);
    String payloadStr = "";
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    Serial.println("MQTT Message: " + topicStr + " -> " + payloadStr);

    // Extract the setting name from topic: side-eye/DEVICE_ID/set/SETTING
    int lastSlash = topicStr.lastIndexOf('/');
    if (lastSlash == -1) return;
    String setting = topicStr.substring(lastSlash + 1);

    bool changed = false;
    if (setting == "brightness") {
        uint8_t val = payloadStr.toInt();
        if (val >= 0 && val <= 255) {
            state.brightness = val;
            display.setBacklight(state, true);
            changed = true;
        }
    } else if (setting == "rotation") {
        int val = payloadStr.toInt();
        if (val == 1 || val == 3) {
            state.rotation = val;
            display.setRotation(val);
            needsStaticDraw = true;
            changed = true;
        }
    } else if (setting == "cycle_duration") {
        long val = payloadStr.toInt();
        if (val >= 1000) {
            state.cycle_duration = val;
            changed = true;
        }
    } else if (setting == "cpu_warning") {
        state.cpu_warning = payloadStr.toInt();
        changed = true;
    } else if (setting == "cpu_critical") {
        state.cpu_critical = payloadStr.toInt();
        changed = true;
    } else if (setting == "ram_warning") {
        state.ram_warning = payloadStr.toInt();
        changed = true;
    } else if (setting == "ram_critical") {
        state.ram_critical = payloadStr.toInt();
        changed = true;
    } else if (setting == "discovery_prefix") {
        network.setDiscoveryPrefix(payloadStr);
        changed = true;
    }

    if (changed) {
        network.saveConfig(state, true);
        network.publishState(state);
        display.showNotification("Settings Updated");
        needsStaticDraw = true; // Refresh UI behind the notification
    }
}

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

void configLoopCallback() {
    static int lastRotation = display.getRotation();
    if (input.update(state, currentPage, lastPageChange, needsStaticDraw, FIRMWARE_VERSION, true)) {
        // Long hold reset not usually handled here, but we check for rotation change
    }
    if (display.getRotation() != lastRotation) {
        lastRotation = display.getRotation();
        display.drawConfigMode(("SideEye-" + deviceID).c_str(), WiFi.softAPIP().toString());
    }
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
        uint8_t old_alert = state.alert_level;
        state.cpu_percent = data["cpu_percent"];
        state.ram_used = data["ram_used"];
        state.ram_total = data["ram_total"];
        state.disk_used = data["disk_used"];
        state.disk_total = data["disk_total"];
        state.net_up = data["net_up"];
        state.net_down = data["net_down"];
        state.net_up_history.push(state.net_up);
        state.net_down_history.push(state.net_down);
        state.uptime = data["uptime"];
        state.thermal_c = data["thermal_c"];
        state.gpu_percent = data["gpu_percent"];
        state.alert_level = data["alert_level"].as<uint8_t>();
        state.has_data = true;
        state.connected = true;

        // Alert Priority: Jump to resources page if alert level increases to Warning or Critical
        if (state.alert_level > 0 && state.alert_level > old_alert) {
            if (currentPage != PAGE_RESOURCES) {
                currentPage = PAGE_RESOURCES;
                needsStaticDraw = true;
                lastPageChange = millis();
            }
        }
        
        if (state.alert_level != old_alert) {
            needsStaticDraw = true;
        }
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

    if (!input.isResetActive()) {
        display.updateDynamicValues(state, currentPage, needsStaticDraw, waitingMessageActive, FIRMWARE_VERSION);
    }
    needsStaticDraw = false;
}

// cppcheck-suppress unusedFunction
void setup() {
    // Immediate backlight activation to confirm hardware is alive
    pinMode(6, OUTPUT);
    digitalWrite(6, HIGH);

    // Set all CS pins HIGH to disable SPI devices initially
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    pinMode(LCD_CS, OUTPUT);
    digitalWrite(LCD_CS, HIGH);

    Serial.begin(115200);
    deviceID = getDeviceID();
    
    // LCD SPI initialization (Bus 1)
    SPI.begin(1, -1, 2, -1); // SCK, MISO (NC), MOSI, CS (Handled by DisplayManager)
    delay(100); 
    
    // SD Initialization (Handled by SyncManager using dedicated SPI)
    syncManager.begin();
    
    display.begin(state);
    input.begin();
    blePresence.begin(deviceID.c_str());
    
    display.drawBootScreen(FIRMWARE_VERSION);
    Serial.printf("\n--- SideEye Firmware v%s starting ---\n", FIRMWARE_VERSION);
    delay(500);

    network.setCallback(onMqttMessage);
    network.begin(deviceID, FIRMWARE_VERSION, state, saveConfigCallback, configModeCallback, configLoopCallback);
    network.saveConfig(state, shouldSaveConfig);
    
    display.drawWiFiOnline();
    delay(200);

    display.drawStaticUI(state, currentPage, FIRMWARE_VERSION);
    display.updateDynamicValues(state, currentPage, true, true, FIRMWARE_VERSION);
    waitingMessageActive = false;
    needsStaticDraw = false;
}

String inputBuffer = "";

// cppcheck-suppress unusedFunction
void loop() {
    if (state.sd_sync_status == "Syncing..." && millis() - lastPageChange > 2000) {
        state.sd_sync_status = "Idle";
        needsStaticDraw = true;
    }

    if (input.update(state, currentPage, lastPageChange, needsStaticDraw, FIRMWARE_VERSION)) {
        network.resetSettings();
    }
    network.update(lastMqttRetry);
    blePresence.update();

    // Timeout for connection status
    static unsigned long lastDataReceived = 0;
    if (state.has_data) {
        lastDataReceived = millis();
        input.notifyActivity();
        state.has_data = false;
    }

    if (state.connected && millis() - lastDataReceived > 10000) {
        state.connected = false;
        needsStaticDraw = true;
    }

    // Logic for state.connected timeout
    // (Handled by handleJson and has_data flag in original, let's keep it simple)

    // Page cycling
    if (input.isScreenOn() && !input.isResetActive()) {
        unsigned long now = millis();
        if (now - lastPageChange > state.cycle_duration) {
            currentPage = static_cast<Page>((currentPage + 1) % NUM_PAGES);
            lastPageChange = now;
            needsStaticDraw = true;
        }
    }

    if (needsStaticDraw && !input.isResetActive()) {
        display.updateDynamicValues(state, currentPage, true, false, FIRMWARE_VERSION);
        needsStaticDraw = false;
    }

    // Refresh banner for flashing effect if in critical alert
    static unsigned long lastFlashUpdate = 0;
    if (state.alert_level >= 2 && millis() - lastFlashUpdate > 500) {
        display.drawBanner("SIDEEYE MONITOR", state.alert_level);
        lastFlashUpdate = millis();
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
