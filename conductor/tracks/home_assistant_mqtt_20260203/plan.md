# Implementation Plan: Integration - Home Assistant via MQTT Discovery

## Phase 1: MQTT Client Infrastructure
- [x] Task: Firmware - Add dependencies to `platformio.ini`. (3cc854b)
    - [x] Add `knolleary/PubSubClient` and `bblanchon/ArduinoJson`.
- [ ] Task: Firmware - Implement Basic MQTT Connection.
    - [ ] Add MQTT client setup to `main.cpp`.
    - [ ] Add configuration fields to `WiFiManager` to collect MQTT credentials.
- [ ] Task: Firmware - Implement Reconnection Logic.
    - [ ] Non-blocking reconnection attempts in the `loop()`.

## Phase 2: Home Assistant Discovery
- [ ] Task: Firmware - Implement Discovery Payload Generation.
    - [ ] Logic to build JSON config payloads for HA sensors.
    - [ ] Publish configs on successful MQTT connection.
- [ ] Task: Firmware - Implement State Reporting.
    - [ ] Publish sensor states (Hostname, IP, MAC) whenever a new valid serial packet is processed.

## Phase 3: Verification & Integration
- [ ] Task: Integration - Verify in Home Assistant.
    - [ ] Connect device to an MQTT broker and check if entities appear in HA "MQTT" integration.
- [ ] Task: Integration - Test Real-time Updates.
    - [ ] Change network info on the host and verify HA updates along with the LCD.
- [ ] Task: Conductor - User Manual Verification 'Home Assistant Integration' (Protocol in workflow.md)
