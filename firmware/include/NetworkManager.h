#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_mac.h>
#include "DisplayManager.h"

#if __has_include("secrets.h")
#include "secrets.h"
#endif

class SideEyeNetworkManager {
public:
    friend class NetworkManagerTest;
    SideEyeNetworkManager() : _mqttClient(_espClient) {
#ifdef MQTT_HOST
        strncpy(mqtt_server, MQTT_HOST, sizeof(mqtt_server) - 1);
#endif
#ifdef MQTT_PORT
        strncpy(mqtt_port, String(MQTT_PORT).c_str(), sizeof(mqtt_port) - 1);
#endif
#ifdef MQTT_USER
        strncpy(mqtt_user, MQTT_USER, sizeof(mqtt_user) - 1);
#endif
#ifdef MQTT_PASS
        strncpy(mqtt_pass, MQTT_PASS, sizeof(mqtt_pass) - 1);
#endif
#ifdef MQTT_TOPIC_PREFIX
        strncpy(mqtt_topic_prefix, MQTT_TOPIC_PREFIX, sizeof(mqtt_topic_prefix) - 1);
#endif
    }

    void begin(const String& deviceID, const char* version, void (*saveCallback)(), void (*configCallback)(WiFiManager*), void (*webServerCallback)()) {
        _deviceID = deviceID;
        _version = version;

        // ... (LittleFS code) ...
        if (LittleFS.begin()) {
            Serial.println("mounted file system");
            if (LittleFS.exists("/config.json")) {
                Serial.println("reading config file");
                File configFile = LittleFS.open("/config.json", "r");
                if (configFile) {
                    JsonDocument json;
                    DeserializationError error = deserializeJson(json, configFile);
                    if (!error) {
                        strcpy(mqtt_server, json["mqtt_server"] | "");
                        strcpy(mqtt_port, json["mqtt_port"] | "1883");
                        strcpy(mqtt_user, json["mqtt_user"] | "");
                        strcpy(mqtt_pass, json["mqtt_pass"] | "");
                        strcpy(mqtt_topic_prefix, json["mqtt_topic_prefix"] | "side-eye");
                    }
                    configFile.close();
                }
            }
        }

        WiFiManager wm;
        wm.setSaveConfigCallback(saveCallback);
        wm.setAPCallback(configCallback);
        wm.setWebServerCallback(webServerCallback);

#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
        wm.preloadWiFi(WIFI_SSID, WIFI_PASSWORD);
#endif

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

        String apName = "SideEye-" + _deviceID;
        if (!wm.autoConnect(apName.c_str())) {
            ESP.restart();
            delay(1000);
        }

        strcpy(mqtt_server, custom_mqtt_server.getValue());
        strcpy(mqtt_port, custom_mqtt_port.getValue());
        strcpy(mqtt_user, custom_mqtt_user.getValue());
        strcpy(mqtt_pass, custom_mqtt_pass.getValue());
        strcpy(mqtt_topic_prefix, custom_mqtt_topic_prefix.getValue());

        if (strlen(mqtt_server) > 0) {
            _mqttClient.setServer(mqtt_server, atoi(mqtt_port));
        }
    }

    void saveConfig(bool shouldSave) {
        if (shouldSave) {
            Serial.println("saving config");
            JsonDocument json;
            json["mqtt_server"] = mqtt_server;
            json["mqtt_port"] = mqtt_port;
            json["mqtt_user"] = mqtt_user;
            json["mqtt_pass"] = mqtt_pass;
            json["mqtt_topic_prefix"] = mqtt_topic_prefix;

            File configFile = LittleFS.open("/config.json", "w");
            if (configFile) {
                serializeJson(json, configFile);
                configFile.close();
            }
        }
    }

    void update(unsigned long& lastMqttRetry) {
        if (strlen(mqtt_server) > 0) {
            if (!_mqttClient.connected()) {
                unsigned long now = millis();
                if (now - lastMqttRetry > 5000) {
                    lastMqttRetry = now;
                    reconnectMQTT();
                }
            }
            _mqttClient.loop();
        }
    }

    void publishState(const SystemState& state) {
        if (!_mqttClient.connected()) return;

        String stateTopic = String(mqtt_topic_prefix) + "/" + _deviceID + "/state";
        JsonDocument doc;
        doc["hostname"] = state.hostname;
        doc["ip"] = state.ip;
        doc["mac"] = state.mac;
        doc["rssi"] = WiFi.RSSI();

        String payload;
        serializeJson(doc, payload);
        _mqttClient.publish(stateTopic.c_str(), payload.c_str());
    }

    void reconnectMQTT() {
        String clientId = "SideEye-" + _deviceID;
        String statusTopic = String(mqtt_topic_prefix) + "/" + _deviceID + "/status";
        
        bool connected = false;
        if (strlen(mqtt_user) > 0) {
            connected = _mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass, statusTopic.c_str(), 1, true, "offline");
        } else {
            connected = _mqttClient.connect(clientId.c_str(), statusTopic.c_str(), 1, true, "offline");
        }

        if (connected) {
            Serial.println("MQTT connected");
            _mqttClient.publish(statusTopic.c_str(), "online", true);
            publishHADiscovery();
        } else {
            Serial.print("failed, rc=");
            Serial.print(_mqttClient.state());
        }
    }

    void publishHADiscovery() {
        String baseTopic = String(mqtt_topic_prefix) + "/" + _deviceID;
        String discoveryPrefix = "homeassistant/sensor/side_eye_" + _deviceID;
        
        struct Sensor {
            const char* name;
            const char* key;
            const char* icon;
        };

        static const Sensor sensors[] = {
            {"Hostname", "hostname", "mdi:label"},
            {"IP Address", "ip", "mdi:ip-network"},
            {"MAC Address", "mac", "mdi:ethernet"},
            {"WiFi RSSI", "rssi", "mdi:wifi"}
        };

        for (const auto& s : sensors) {
            JsonDocument doc;
            doc["name"] = String("SideEye ") + _deviceID + " " + s.name;
            doc["state_topic"] = baseTopic + "/state";
            doc["value_template"] = String("{{ value_json.") + s.key + " }}";
            doc["unique_id"] = String("side_eye_") + _deviceID + "_" + s.key;
            doc["icon"] = s.icon;
            
            JsonObject dev = doc["device"].to<JsonObject>();
            dev["identifiers"][0] = String("side_eye_") + _deviceID;
            dev["name"] = String("SideEye ") + _deviceID;
            dev["model"] = "ESP32-C6 GEEK";
            dev["manufacturer"] = "Waveshare";
            dev["sw_version"] = _version;

            String topic = discoveryPrefix + "_" + s.key + "/config";
            String payload;
            serializeJson(doc, payload);
            _mqttClient.publish(topic.c_str(), payload.c_str(), true);
        }

        JsonDocument binDoc;
        binDoc["name"] = String("SideEye ") + _deviceID + " Status";
        binDoc["state_topic"] = baseTopic + "/status";
        binDoc["unique_id"] = String("side_eye_") + _deviceID + "_status";
        binDoc["device_class"] = "connectivity";
        binDoc["payload_on"] = "online";
        binDoc["payload_off"] = "offline";

        JsonObject binDev = binDoc["device"].to<JsonObject>();
        binDev["identifiers"][0] = String("side_eye_") + _deviceID;

        String binTopic = "homeassistant/binary_sensor/side_eye_" + _deviceID + "_status/config";
        String binPayload;
        serializeJson(binDoc, binPayload);
        _mqttClient.publish(binTopic.c_str(), binPayload.c_str(), true);
    }

    void resetSettings() {
        WiFiManager wm;
        wm.resetSettings();
        if (LittleFS.begin()) {
            LittleFS.remove("/config.json");
        }
        Serial.println("Settings reset, restarting...");
        delay(1000);
        ESP.restart();
    }

private:
    WiFiClient _espClient;
    PubSubClient _mqttClient;
    String _deviceID;
    String _version;

    char mqtt_server[40] = "";
    char mqtt_port[6] = "1883";
    char mqtt_user[40] = "";
    char mqtt_pass[40] = "";
    char mqtt_topic_prefix[40] = "side-eye";
};

#endif
