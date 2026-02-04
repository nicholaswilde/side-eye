#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <esp_mac.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include "catppuccin_colors.h"

/* 
 * Waveshare ESP32-C6-GEEK Configuration
 */
#define LCD_SCK 1
#define LCD_MOSI 2
#define LCD_MISO -1
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6
#define BTN_PIN 9

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.0.0-unknown"
#endif

class Button {
public:
    enum Event { NONE, CLICK, DOUBLE_CLICK, HOLD };

    Button(int pin) : _pin(pin) {}

    void begin() {
        pinMode(_pin, INPUT_PULLUP);
        _lastState = digitalRead(_pin);
    }

    Event update() {
        int currentState = digitalRead(_pin);
        unsigned long now = millis();
        Event event = NONE;

        if (currentState != _lastState) {
            _lastDebounceTime = now;
        }

        if ((now - _lastDebounceTime) > _debounceDelay) {
            if (currentState != _stableState) {
                _stableState = currentState;
                if (_stableState == LOW) { // Pressed
                    _pressStartTime = now;
                    _holdReported = false;
                    if (now - _lastClickTime < _doubleClickDelay) {
                        _pendingClick = false;
                        event = DOUBLE_CLICK;
                    }
                } else { // Released
                    if (!_holdReported && event != DOUBLE_CLICK) {
                        _pendingClick = true;
                        _lastClickTime = now;
                    }
                }
            }
        }

        if (_stableState == LOW && !_holdReported) {
            if (now - _pressStartTime > _holdDelay) {
                event = HOLD;
                _holdReported = true;
                _pendingClick = false;
            }
        }

        if (_pendingClick && (now - _lastClickTime > _doubleClickDelay)) {
            _pendingClick = false;
            event = CLICK;
        }

        _lastState = currentState;
        return event;
    }

private:
    int _pin;
    int _lastState = HIGH;
    int _stableState = HIGH;
    unsigned long _lastDebounceTime = 0;
    unsigned long _debounceDelay = 50;
    unsigned long _pressStartTime = 0;
    unsigned long _holdDelay = 800;
    unsigned long _lastClickTime = 0;
    unsigned long _doubleClickDelay = 300;
    bool _holdReported = false;
    bool _pendingClick = false;
};

Button button(BTN_PIN);
bool isScreenOn = true;
unsigned long lastActivityTime = 0;
const unsigned long AUTO_OFF_DELAY = 60000; // 1 minute

struct SystemState {
    String hostname = "Unknown";
    String ip = "No IP";
    String mac = "No MAC";
    String os = "Unknown";
    String user = "Unknown";
    float cpu_percent = 0;
    uint64_t ram_used = 0;
    uint64_t ram_total = 0;
    uint64_t disk_used = 0;
    uint64_t disk_total = 0;
    uint64_t net_up = 0;
    uint64_t net_down = 0;
    uint64_t uptime = 0;
    bool has_data = false;
    bool connected = false;
};

SystemState state;
SystemState last_state;
int current_rotation = 1; // 1 = 90 deg, 3 = 270 deg (180 deg flip)
bool needs_static_draw = true;
bool waiting_message_active = true;

enum Page {
    PAGE_IDENTITY,
    PAGE_RESOURCES,
    PAGE_STATUS,
    NUM_PAGES
};

Page currentPage = PAGE_IDENTITY;
unsigned long lastPageChange = 0;
const unsigned long PAGE_DURATION = 5000; // 5 seconds per page

// MQTT Settings
char mqtt_server[40];
char mqtt_port[6] = "1883";
char mqtt_user[40];
char mqtt_pass[40];
char mqtt_topic_prefix[40] = "side-eye";

bool shouldSaveConfig = false;

// Callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String deviceID = "";

void publishHADiscovery() {
    String baseTopic = String(mqtt_topic_prefix) + "/" + deviceID;
    String discoveryPrefix = "homeassistant/sensor/side_eye_" + deviceID;
    
    struct Sensor {
        const char* name;
        const char* key;
        const char* icon;
    };

    Sensor sensors[] = {
        {"Hostname", "hostname", "mdi:label"},
        {"IP Address", "ip", "mdi:ip-network"},
        {"MAC Address", "mac", "mdi:ethernet"},
        {"WiFi RSSI", "rssi", "mdi:wifi"}
    };

    for (const auto& s : sensors) {
        JsonDocument doc;
        doc["name"] = String("SideEye ") + deviceID + " " + s.name;
        doc["state_topic"] = baseTopic + "/state";
        doc["value_template"] = String("{{ value_json.") + s.key + " }}";
        doc["unique_id"] = String("side_eye_") + deviceID + "_" + s.key;
        doc["icon"] = s.icon;
        
        JsonObject dev = doc["device"].to<JsonObject>();
        dev["identifiers"][0] = String("side_eye_") + deviceID;
        dev["name"] = String("SideEye ") + deviceID;
        dev["model"] = "ESP32-C6 GEEK";
        dev["manufacturer"] = "Waveshare";
        dev["sw_version"] = FIRMWARE_VERSION;

        String topic = discoveryPrefix + "_" + s.key + "/config";
        String payload;
        serializeJson(doc, payload);
        mqttClient.publish(topic.c_str(), payload.c_str(), true);
    }

    // Availability Binary Sensor
    JsonDocument binDoc;
    binDoc["name"] = String("SideEye ") + deviceID + " Status";
    binDoc["state_topic"] = baseTopic + "/status";
    binDoc["unique_id"] = String("side_eye_") + deviceID + "_status";
    binDoc["device_class"] = "connectivity";
    binDoc["payload_on"] = "online";
    binDoc["payload_off"] = "offline";

    JsonObject binDev = binDoc["device"].to<JsonObject>();
    binDev["identifiers"][0] = String("side_eye_") + deviceID;

    String binTopic = "homeassistant/binary_sensor/side_eye_" + deviceID + "_status/config";
    String binPayload;
    serializeJson(binDoc, binPayload);
    mqttClient.publish(binTopic.c_str(), binPayload.c_str(), true);
}

void reconnectMQTT() {
    // Attempt to connect
    String clientId = "SideEye-" + deviceID;
    String statusTopic = String(mqtt_topic_prefix) + "/" + deviceID + "/status";
    
    bool connected = false;
    if (strlen(mqtt_user) > 0) {
        connected = mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass, statusTopic.c_str(), 1, true, "offline");
    } else {
        connected = mqttClient.connect(clientId.c_str(), statusTopic.c_str(), 1, true, "offline");
    }

    if (connected) {
        Serial.println("MQTT connected");
        mqttClient.publish(statusTopic.c_str(), "online", true);
        publishHADiscovery();
    } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
    }
}

void publishMQTTState() {
    if (!mqttClient.connected()) return;

    String stateTopic = String(mqtt_topic_prefix) + "/" + deviceID + "/state";
    JsonDocument doc;
    doc["hostname"] = state.hostname;
    doc["ip"] = state.ip;
    doc["mac"] = state.mac;
    doc["rssi"] = WiFi.RSSI();

    String payload;
    serializeJson(doc, payload);
    mqttClient.publish(stateTopic.c_str(), payload.c_str());
}

String getDeviceID() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char id[7];
    snprintf(id, sizeof(id), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(id);
}

Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);

// Centering 135x240 Portrait area in 240x320 RAM, then rotating to Landscape
Arduino_GFX *gfx = new Arduino_ST7789(
    bus, LCD_RST, 0 /* rotation */, true /* IPS */,
    135 /* width */, 240 /* height */,
    52 /* col offset 1 */, 40 /* row offset 1 */,
    53 /* col offset 2 */, 40 /* row offset 2 */
);

const int start_x = 10;
const int start_y = 30;
const int line_h = 12;
const int value_x = 55;

void drawWiFiStatus() {
    int x = (current_rotation == 1) ? 225 : 15;
    int y = 10;
    if (WiFi.status() == WL_CONNECTED) {
        gfx->fillCircle(x, y, 3, CATPPUCCIN_GREEN);
    } else {
        gfx->fillCircle(x, y, 3, CATPPUCCIN_RED);
    }
}

void drawProgressBar(int x, int y, int w, int h, float percent, uint16_t color) {
    gfx->drawRect(x, y, w, h, CATPPUCCIN_SURFACE0);
    int fill_w = (int)((w - 2) * (percent / 100.0));
    if (fill_w < 0) fill_w = 0;
    if (fill_w > w - 2) fill_w = w - 2;
    gfx->fillRect(x + 1, y + 1, w - 2, h - 2, CATPPUCCIN_BASE);
    gfx->fillRect(x + 1, y + 1, fill_w, h - 2, color);
}

void drawBanner(const char* title) {
    gfx->fillRect(0, 0, 240, 20, CATPPUCCIN_MAUVE);
    gfx->setTextColor(CATPPUCCIN_CRUST);
    gfx->setTextSize(1);
    
    int16_t x1, y1;
    uint16_t w, h;
    gfx->getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    gfx->setCursor((240 - w) / 2, 6);
    gfx->println(title);
}

void drawIdentityPage(bool labelsOnly) {
    if (labelsOnly) {
        gfx->setTextColor(CATPPUCCIN_BLUE);
        gfx->setCursor(start_x, start_y + line_h * 1.5);
        gfx->print("Host: ");

        gfx->setCursor(start_x, start_y + line_h * 2.5);
        gfx->setTextColor(CATPPUCCIN_GREEN);
        gfx->print("IP:   ");

        gfx->setCursor(start_x, start_y + line_h * 3.5);
        gfx->setTextColor(CATPPUCCIN_FLAMINGO);
        gfx->print("MAC:  ");
    } else {
        gfx->setTextColor(CATPPUCCIN_TEXT);
        
        gfx->fillRect(value_x, (int)(start_y + line_h * 1.5), 180, 8, CATPPUCCIN_BASE);
        gfx->setCursor(value_x, (int)(start_y + line_h * 1.5));
        gfx->println(state.hostname);

        gfx->fillRect(value_x, (int)(start_y + line_h * 2.5), 180, 8, CATPPUCCIN_BASE);
        gfx->setCursor(value_x, (int)(start_y + line_h * 2.5));
        gfx->println(state.ip);

        gfx->fillRect(value_x, (int)(start_y + line_h * 3.5), 180, 8, CATPPUCCIN_BASE);
        gfx->setCursor(value_x, (int)(start_y + line_h * 3.5));
        gfx->println(state.mac);
    }
}

void drawResourcesPage(bool labelsOnly) {
    if (labelsOnly) {
        gfx->setCursor(start_x, start_y + line_h * 1.5);
        gfx->setTextColor(CATPPUCCIN_PEACH);
        gfx->print("CPU:  ");

        gfx->setCursor(start_x, start_y + line_h * 3.5);
        gfx->setTextColor(CATPPUCCIN_SAPPHIRE);
        gfx->print("RAM:  ");
    } else {
        gfx->setTextColor(CATPPUCCIN_TEXT);

        // CPU
        gfx->fillRect(value_x, (int)(start_y + line_h * 1.5), 100, 8, CATPPUCCIN_BASE);
        gfx->setCursor(value_x, (int)(start_y + line_h * 1.5));
        gfx->print(state.cpu_percent, 1);
        gfx->println("%");
        uint16_t cpu_col = (state.cpu_percent > 80) ? CATPPUCCIN_RED : (state.cpu_percent > 50) ? CATPPUCCIN_YELLOW : CATPPUCCIN_GREEN;
        drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, state.cpu_percent, cpu_col);

        // RAM
        gfx->fillRect(value_x, (int)(start_y + line_h * 3.5), 180, 8, CATPPUCCIN_BASE);
        gfx->setCursor(value_x, (int)(start_y + line_h * 3.5));
        gfx->printf("%llu / %llu MB", state.ram_used / 1024 / 1024, state.ram_total / 1024 / 1024);
        float ram_p = (state.ram_total > 0) ? (float)state.ram_used / state.ram_total * 100.0 : 0;
        uint16_t ram_col = (ram_p > 80) ? CATPPUCCIN_RED : (ram_p > 50) ? CATPPUCCIN_YELLOW : CATPPUCCIN_GREEN;
        drawProgressBar(start_x, start_y + line_h * 4.5, 220, 8, ram_p, ram_col);
    }
}

void drawStatusPage(bool labelsOnly) {
    if (labelsOnly) {
        gfx->setCursor(start_x, start_y + line_h * 1.5);
        gfx->setTextColor(CATPPUCCIN_TEAL);
        gfx->print("Disk: ");

        gfx->setCursor(start_x, start_y + line_h * 3.5);
        gfx->setTextColor(CATPPUCCIN_SUBTEXT0);
        gfx->print("Uptime: ");
    } else {
        gfx->setTextColor(CATPPUCCIN_TEXT);

        // Disk
        gfx->fillRect(value_x, (int)(start_y + line_h * 1.5), 180, 8, CATPPUCCIN_BASE);
        gfx->setCursor(value_x, (int)(start_y + line_h * 1.5));
        gfx->printf("%llu / %llu GB", state.disk_used / 1024 / 1024 / 1024, state.disk_total / 1024 / 1024 / 1024);
        float disk_p = (state.disk_total > 0) ? (float)state.disk_used / state.disk_total * 100.0 : 0;
        uint16_t disk_col = (disk_p > 80) ? CATPPUCCIN_RED : (disk_p > 50) ? CATPPUCCIN_YELLOW : CATPPUCCIN_GREEN;
        drawProgressBar(start_x, start_y + line_h * 2.5, 220, 8, disk_p, disk_col);

        // Uptime
        gfx->fillRect(value_x + 20, (int)(start_y + line_h * 3.5), 160, 8, CATPPUCCIN_BASE);
        uint32_t h_up = state.uptime / 3600;
        uint32_t m_up = (state.uptime % 3600) / 60;
        gfx->setCursor(value_x + 20, (int)(start_y + line_h * 3.5));
        gfx->printf("%uh %um", h_up, m_up);
    }
}

void drawStaticUI() {
    gfx->fillScreen(CATPPUCCIN_BASE);
    drawBanner("SIDEEYE MONITOR");
    drawWiFiStatus();

    gfx->setTextSize(1);
    
    gfx->setTextColor(CATPPUCCIN_YELLOW);
    gfx->setCursor(start_x, start_y);
    gfx->print("Status:");

    if (state.connected) {
        switch (currentPage) {
            case PAGE_IDENTITY: drawIdentityPage(true); break;
            case PAGE_RESOURCES: drawResourcesPage(true); break;
            case PAGE_STATUS: drawStatusPage(true); break;
            default: break;
        }
    }

    // Version back in bottom right corner
    gfx->setTextColor(CATPPUCCIN_SURFACE1);
    gfx->setCursor(200, 120);
    gfx->print(FIRMWARE_VERSION);

    needs_static_draw = false;
}

void updateDynamicValues() {
    bool force_redraw = needs_static_draw || waiting_message_active;

    if (force_redraw) {
        drawStaticUI();
        waiting_message_active = false;
    }

    gfx->setTextSize(1);

    // Status value
    gfx->fillRect(value_x, start_y, 140, 8, CATPPUCCIN_BASE);
    gfx->setCursor(value_x, start_y);
    if (state.connected) {
        gfx->setTextColor(CATPPUCCIN_GREEN);
        gfx->println("Connected");

        switch (currentPage) {
            case PAGE_IDENTITY: drawIdentityPage(false); break;
            case PAGE_RESOURCES: drawResourcesPage(false); break;
            case PAGE_STATUS: drawStatusPage(false); break;
            default: break;
        }
    } else {
        gfx->setTextColor(CATPPUCCIN_PEACH);
        gfx->println("Waiting...");
    }

    last_state = state;
}

void configModeCallback (WiFiManager *myWiFiManager) {
    gfx->fillScreen(CATPPUCCIN_BASE);
    drawBanner("SETUP MODE");
    
    gfx->setTextColor(CATPPUCCIN_TEXT);
    gfx->setTextSize(1);
    gfx->setCursor(15, 40);
    gfx->println("Connect to WiFi AP:");
    
    gfx->setTextColor(CATPPUCCIN_YELLOW);
    gfx->setCursor(15, 55);
    String apName = "SideEye-" + deviceID;
    gfx->println(apName);
    
    gfx->setTextColor(CATPPUCCIN_TEXT);
    gfx->setCursor(15, 80);
    gfx->println("Then visit:");
    gfx->setTextColor(CATPPUCCIN_GREEN);
    gfx->println(WiFi.softAPIP().toString());
}

void setup() {
    Serial.begin(115200);
    deviceID = getDeviceID();
    String apName = "SideEye-" + deviceID;
    
    button.begin();
    lastActivityTime = millis();
    
    pinMode(BTN_PIN, INPUT_PULLUP);

    if (LCD_BL >= 0) {
        pinMode(LCD_BL, OUTPUT);
        digitalWrite(LCD_BL, HIGH);
    }

    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }
    
    gfx->setRotation(current_rotation); 
    gfx->fillScreen(CATPPUCCIN_BASE);

    // Read configuration from LittleFS
    if (LittleFS.begin()) {
        Serial.println("mounted file system");
        if (LittleFS.exists("/config.json")) {
            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = LittleFS.open("/config.json", "r");
            if (configFile) {
                Serial.println("opened config file");
                JsonDocument json;
                DeserializationError error = deserializeJson(json, configFile);
                if (!error) {
                    Serial.println("\nparsed json");
                    strcpy(mqtt_server, json["mqtt_server"] | "");
                    strcpy(mqtt_port, json["mqtt_port"] | "1883");
                    strcpy(mqtt_user, json["mqtt_user"] | "");
                    strcpy(mqtt_pass, json["mqtt_pass"] | "");
                    strcpy(mqtt_topic_prefix, json["mqtt_topic_prefix"] | "side-eye");
                } else {
                    Serial.println("failed to load json config");
                }
                configFile.close();
            }
        }
    } else {
        Serial.println("failed to mount FS");
    }
    
    WiFiManager wm;
    wm.setSaveConfigCallback(saveConfigCallback);
    wm.setAPCallback(configModeCallback);

    // Custom MQTT parameters
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 40);
    WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 40);
    WiFiManagerParameter custom_mqtt_topic_prefix("prefix", "topic prefix", mqtt_topic_prefix, 40);

    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_mqtt_user);
    wm.addParameter(&custom_mqtt_pass);
    wm.addParameter(&custom_mqtt_topic_prefix);
    
    drawBanner("BOOTING...");
    gfx->setCursor(15, start_y);
    gfx->setTextColor(CATPPUCCIN_SUBTEXT0);
    gfx->printf("v%s", FIRMWARE_VERSION);
    Serial.printf("\n--- SideEye Firmware v%s starting ---\n", FIRMWARE_VERSION);
    Serial.printf("Device ID: %s\n", deviceID.c_str());
    Serial.printf("AP Name:   %s\n", apName.c_str());
    
    // Hold boot screen for a moment so version is visible
    delay(2000);

    if (!wm.autoConnect(apName.c_str())) {
        ESP.restart();
        delay(1000);
    }

    // Read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_pass, custom_mqtt_pass.getValue());
    strcpy(mqtt_topic_prefix, custom_mqtt_topic_prefix.getValue());

    // Save the custom parameters to FS
    if (shouldSaveConfig) {
        Serial.println("saving config");
        JsonDocument json;
        json["mqtt_server"] = mqtt_server;
        json["mqtt_port"] = mqtt_port;
        json["mqtt_user"] = mqtt_user;
        json["mqtt_pass"] = mqtt_pass;
        json["mqtt_topic_prefix"] = mqtt_topic_prefix;

        File configFile = LittleFS.open("/config.json", "w");
        if (!configFile) {
            Serial.println("failed to open config file for writing");
        }

        serializeJson(json, configFile);
        configFile.close();
    }
    
    gfx->fillScreen(CATPPUCCIN_BASE);
    drawBanner("CONNECTED");
    gfx->setCursor(15, start_y);
    gfx->setTextColor(CATPPUCCIN_GREEN);
    gfx->println("WiFi Online!");
    delay(1000);

    // Setup MQTT client
    if (strlen(mqtt_server) > 0) {
        mqttClient.setServer(mqtt_server, atoi(mqtt_port));
    }

    drawStaticUI();
    updateDynamicValues();
}

String inputBuffer = "";

void handleJson(String json) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        // Serial.print("JSON Error: ");
        // Serial.println(error.c_str());
        return;
    }

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
    }

    if (state.connected && !was_connected) {
        needs_static_draw = true;
    }

    if (state.connected) {
        publishMQTTState();
    }

    if (strcmp(type, "GetVersion") == 0) {
        JsonDocument res;
        res["type"] = "Version";
        res["version"] = FIRMWARE_VERSION;
        serializeJson(res, Serial);
        Serial.println();
    }

    updateDynamicValues();
}

void loop() {
    Button::Event ev = button.update();
    if (ev == Button::CLICK) {
        lastActivityTime = millis();
        if (!isScreenOn) {
            isScreenOn = true;
            digitalWrite(LCD_BL, HIGH);
        } else {
            // Next page
            currentPage = static_cast<Page>((currentPage + 1) % NUM_PAGES);
            lastPageChange = millis();
            needs_static_draw = true;
            updateDynamicValues();
        }
    } else if (ev == Button::DOUBLE_CLICK) {
        lastActivityTime = millis();
        current_rotation = (current_rotation == 1) ? 3 : 1;
        gfx->setRotation(current_rotation);
        needs_static_draw = true;
        updateDynamicValues();
    } else if (ev == Button::HOLD) {
        lastActivityTime = millis();
        isScreenOn = !isScreenOn;
        digitalWrite(LCD_BL, isScreenOn ? HIGH : LOW);
    }

    static unsigned long lastWiFiCheck = 0;
    static unsigned long lastDataTime = 0;
    static unsigned long lastMqttRetry = 0;

    if (strlen(mqtt_server) > 0) {
        if (!mqttClient.connected()) {
            unsigned long now = millis();
            if (now - lastMqttRetry > 5000) {
                lastMqttRetry = now;
                reconnectMQTT();
            }
        }
        mqttClient.loop();
    }

    if (state.has_data) {
        lastDataTime = millis();
        lastActivityTime = lastDataTime; // Data from host counts as activity
        if (!isScreenOn) {
            isScreenOn = true;
            digitalWrite(LCD_BL, HIGH);
        }
        state.has_data = false;
    }

    // Timeout for connection status
    if (state.connected && (millis() - lastDataTime > 5000)) {
        state.connected = false;
        needs_static_draw = true;
        updateDynamicValues();
    }

    // Auto-off for screen
    if (isScreenOn && (millis() - lastActivityTime > AUTO_OFF_DELAY)) {
        isScreenOn = false;
        digitalWrite(LCD_BL, LOW);
    }

    if (millis() - lastWiFiCheck > 10000) {
        drawWiFiStatus();
        lastWiFiCheck = millis();
    }

    // Page cycling
    if (state.connected && isScreenOn) {
        unsigned long now = millis();
        if (now - lastPageChange > PAGE_DURATION) {
            currentPage = static_cast<Page>((currentPage + 1) % NUM_PAGES);
            lastPageChange = now;
            needs_static_draw = true;
            updateDynamicValues();
        }
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