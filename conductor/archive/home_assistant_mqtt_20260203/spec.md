# Specification: Integration - Home Assistant via MQTT Discovery

## Context
To integrate SideEye into a smart home ecosystem, the ESP32 device needs to communicate with an MQTT broker. By using Home Assistant (HA) MQTT Discovery, the device can automatically appear as a set of sensors in Home Assistant without manual YAML configuration.

## Goals
1.  **MQTT Connectivity:** The device must connect to a user-defined MQTT broker.
2.  **HA Discovery:** Automatically register sensors for the host's info (Hostname, IP, MAC) and the device's own health (Wi-Fi RSSI, Uptime).
3.  **Real-time Updates:** Push data to HA whenever it is updated via Serial from the host.

## Detailed Requirements

### Firmware (C++)
-   **Dependencies:**
    -   `knolleary/PubSubClient` (MQTT)
    -   `bblanchon/ArduinoJson` (Discovery payloads)
-   **MQTT Configuration:**
    -   Integrate MQTT settings into `WiFiManager` (Broker IP, Port, User, Password, Topic Prefix).
-   **MQTT Discovery:**
    -   On boot/connection, publish discovery payloads to `homeassistant/sensor/side_eye_<mac>/.../config`.
-   **Entities to Expose:**
    -   `Hostname` (Sensor)
    -   `Host IP` (Sensor)
    -   `Host MAC` (Sensor)
    -   `ESP32 WiFi RSSI` (Diagnostic Sensor)
    -   `Device Status` (Binary Sensor: Online/Offline via LWT)

### Communication Flow
1.  Host sends `HOSTNAME|IP|MAC\n` to ESP32 via Serial.
2.  ESP32 parses data and updates LCD.
3.  ESP32 publishes a JSON payload to the MQTT state topic.
4.  Home Assistant updates the dashboard entities.

## Non-Functional Requirements
-   **Reliability:** Use MQTT Last Will and Testament (LWT) to report an "unavailable" status in HA if the device loses power.
-   **Scalability:** Topic structure should follow a standard like `side-eye/<mac>/state`.
