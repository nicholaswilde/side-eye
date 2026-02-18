# Implementation Plan - OTA Firmware Updates

This plan outlines the steps to implement multi-channel Over-the-Air (OTA) firmware updates, including GitHub integration, a local web server, and MQTT control.

## Phase 1: Local Web Update & Progress UI [checkpoint: a4172dd]
- [x] Task: Implement Web Update Server [3993b51]
    - [x] Integrate `ElegantOTA` or `ESP8266HTTPUpdateServer` (adapted for ESP32) into `SideEyeNetworkManager`.
    - [x] Create a `/update` route for binary uploads.
- [x] Task: Create OTA Progress UI [3993b51]
    - [x] Add `DisplayManager::drawUpdateScreen(int progress, const char* status)` to show a visual progress bar.
    - [x] Hook into OTA callbacks to update the LCD in real-time.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Local Web Update & Progress UI' (Protocol in workflow.md)

## Phase 2: GitHub & MQTT Integration [checkpoint: 05f6b74]
- [x] Task: Implement GitHub Version Check [939e6f0]
    - [x] Create a service in `SideEyeNetworkManager` to query the GitHub Tags/Releases API using `HTTPClient`.
    - [x] Compare the local `FIRMWARE_VERSION` with the latest remote tag.
- [x] Task: Implement MQTT Triggered Update [e24085d]
    - [x] Add a handler for the `ota_url` topic in the MQTT callback.
    - [x] Use `httpUpdate.update(client, url)` to perform the flash from a remote URL.
- [x] Task: Conductor - User Manual Verification 'Phase 2: GitHub & MQTT Integration' (Protocol in workflow.md)

## Phase 3: Safety & Persistence [checkpoint: 5a79ecc]
- [x] Task: Verify Settings Retention [28fb272]
    - [x] Perform end-to-end tests to ensure `LittleFS` is not wiped during the update process.
    - [x] Add a "Post-Update" flag to show a "Update Successful" screen on the first boot after a flash.
- [x] Task: Final Refinement [5a79ecc]
    - [x] Add error handling for low-memory or failed downloads.
    - [x] Ensure the device correctly handles "Waiting for Host" state if the host agent is active during an update.
- [x] Task: Conductor - User Manual Verification 'Phase 3: Safety & Persistence' (Protocol in workflow.md)
