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

## Phase 2: Communication & Host Logic
- [~] Task: Expand Serial & MQTT Protocols
    - [ ] Add the `Presence` message type to the firmware's JSON handler.
    - [ ] Implement MQTT status publishing for BLE state (`Disabled`, `Scanning`, `Connected`).
    - [ ] Implement MQTT publishing for presence state (`Present`, `Away`).
- [ ] Task: Update Host Agent (Rust)
    - [ ] Update the host agent's JSON handling to recognize the `Presence` message.
    - [ ] Integrate a desktop notification crate (e.g., `notify-rust`) to show presence-based alerts.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Communication & Host Logic' (Protocol in workflow.md)

## Phase 3: Configuration & Testing
- [ ] Task: Implement BLE Config Persistence & Control
    - [ ] Update `config.json` in LittleFS to store target BLE MAC addresses and the enable/disable toggle state.
    - [ ] Add MQTT `set` topic `ble_presence_enabled` to remotely toggle the feature.
    - [ ] Add MQTT `set` topics to allow remote configuration of the presence target (MAC/UUID).
- [ ] Task: End-to-End Testing
    - [ ] Verify that signal drops don't cause notification spam.
    - [ ] Verify that the host agent correctly identifies presence from multiple SideEye units.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Configuration & Testing' (Protocol in workflow.md)
