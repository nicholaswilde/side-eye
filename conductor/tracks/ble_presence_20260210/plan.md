# Implementation Plan - BLE Presence Detection

This plan outlines the steps to implement BLE-based presence detection using the ESP32-C6 and integrate it with the SideEye host agent.

## Phase 1: ESP32 BLE Infrastructure [checkpoint: 89fe4df]
- [x] Task: Implement BLE Scanner & Service (d27e33f)
    - [x] Initialize `NimBLE-Arduino` on the ESP32-C6. (Note: Switched to standard BLE due to compatibility)
    - [x] Create a scanner task to look for configured MAC addresses.
    - [x] Create a BLE Peripheral with a custom Presence Service and Characteristic.
- [x] Task: Implement Presence Smoothing (ee23f7d)
    - [x] Add logic to maintain a "detected" state for X seconds after the last seen advertisement to prevent flickering.
- [x] Task: Conductor - User Manual Verification 'Phase 1: ESP32 BLE Infrastructure' (Protocol in workflow.md)

## Phase 2: Communication & Host Logic [checkpoint: a3e0ead]
- [x] Task: Expand Serial & MQTT Protocols (19c3053)
    - [x] Add the `Presence` message type to the firmware's JSON handler.
    - [x] Implement MQTT status publishing for BLE state (`Disabled`, `Scanning`, `Connected`).
    - [x] Implement MQTT publishing for presence state (`Present`, `Away`).
- [x] Task: Update Host Agent (Rust) (9025c5f)
    - [x] Update the host agent's JSON handling to recognize the `Presence` message.
    - [x] Integrate a desktop notification crate (e.g., `notify-rust`) to show presence-based alerts.
- [x] Task: Conductor - User Manual Verification 'Phase 2: Communication & Host Logic' (Protocol in workflow.md)

## Phase 3: Configuration & Testing [checkpoint: a394223]
- [x] Task: Implement BLE Config Persistence & Control (7e86271)
    - [x] Update `config.json` in LittleFS to store target BLE MAC addresses and the enable/disable toggle state.
    - [x] Add MQTT `set` topic `ble_presence_enabled` to remotely toggle the feature.
    - [x] Add MQTT `set` topics to allow remote configuration of the presence target (MAC/UUID).
- [x] Task: End-to-End Testing (9ab6c2c)
    - [x] Verify that signal drops don't cause notification spam.
    - [x] Verify that the host agent correctly identifies presence from multiple SideEye units.
- [x] Task: Conductor - User Manual Verification 'Phase 3: Configuration & Testing' (Protocol in workflow.md)
