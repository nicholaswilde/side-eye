# Specification - BLE Presence Detection

## Overview
Leverage the ESP32-C6's Bluetooth 5.0 capabilities to detect the proximity of a user's device (phone, smartwatch, or BLE beacon). This information is communicated to the host machine to provide presence-aware notifications and state updates.

## Functional Requirements
- **Presence Detection:**
  - **Scanner Mode:** The device shall scan for pre-configured BLE MAC addresses or iBeacon UUIDs.
  - **Peripheral Mode:** The device shall allow a BLE client to connect and maintain a "presence session".
- **Communication Interface:**
  - **Host Serial:** The device shall send a `{"type": "Presence", "data": {"status": true/false}}` message to the host agent via USB serial.
  - **MQTT:** The device shall publish to `side-eye/DEVICE_ID/state/presence` (`present`/`away`).
  - **BLE Service:** A custom BLE Service with a `PresenceState` characteristic (read/notify) shall be exposed.
- **Control & Status (MQTT):**
  - **Control:** The device shall subscribe to `side-eye/DEVICE_ID/set/ble_presence_enabled` (true/false) to toggle the feature remotely.
  - **Status Reporting:** The device shall publish its current BLE state (e.g., `Disabled`, `Scanning`, `Connected`) to `side-eye/DEVICE_ID/state/ble_status`.
- **Host Agent Logic:**
  - The Rust host agent shall listen for `Presence` serial messages.
  - On "Presence Lost", the agent shall trigger a desktop notification using native system libraries.

## Non-Functional Requirements
- **Latency:** Presence detection state changes should be reported within 2 seconds of the event.
- **Reliability:** Implement a "smoothing" algorithm to prevent flickering presence status due to signal noise.

## Acceptance Criteria
- [ ] Bringing a registered BLE device into range updates the status on the SideEye LCD and sends a serial message.
- [ ] Disconnecting or moving the BLE device out of range triggers a "User Away" notification on the Linux desktop.
- [ ] The presence status is accurately reflected in Home Assistant via MQTT.
- [ ] Toggling the feature via MQTT correctly starts or stops the BLE hardware on the device.
