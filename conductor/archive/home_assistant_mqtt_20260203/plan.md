# Implementation Plan: Integration - Home Assistant via MQTT Discovery

## Phase 1: MQTT Client Infrastructure
- [x] Task: Firmware - Add dependencies to `platformio.ini`. (3cc854b)
    - [x] Add `knolleary/PubSubClient` and `bblanchon/ArduinoJson`.
- [x] Task: Firmware - Implement Basic MQTT Connection. (79e1a48)
    - [x] Add MQTT client setup to `main.cpp`.
    - [x] Add configuration fields to `WiFiManager` to collect MQTT credentials.
- [x] Task: Firmware - Implement Reconnection Logic. (79e1a48)
    - [x] Non-blocking reconnection attempts in the `loop()`.

## Phase 2: Home Assistant Discovery
- [x] Task: Firmware - Implement Discovery Payload Generation. (900e5a0)
    - [x] Logic to build JSON config payloads for HA sensors.
    - [x] Publish configs on successful MQTT connection.
- [x] Task: Firmware - Implement State Reporting. (900e5a0)
    - [x] Publish sensor states (Hostname, IP, MAC) whenever a new valid serial packet is processed.

## Phase 3: Verification & Integration
- [x] Task: Integration - Verify in Home Assistant. (900e5a0)
    - [x] Connect device to an MQTT broker and check if entities appear in HA "MQTT" integration.
- [x] Task: Integration - Test Real-time Updates. (900e5a0)
    - [x] Change network info on the host and verify HA updates along with the LCD.
- [x] Task: Conductor - User Manual Verification 'Home Assistant Integration' (Protocol in workflow.md) (900e5a0)
