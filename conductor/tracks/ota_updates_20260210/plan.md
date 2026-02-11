# Implementation Plan - OTA Firmware Updates

This plan outlines the steps to implement multi-channel Over-the-Air (OTA) firmware updates, including GitHub integration, a local web server, and MQTT control.

## Phase 1: Local Web Update & Progress UI
- [ ] Task: Implement Web Update Server
    - [ ] Integrate `ElegantOTA` or `ESP8266HTTPUpdateServer` (adapted for ESP32) into `SideEyeNetworkManager`.
    - [ ] Create a `/update` route for binary uploads.
- [ ] Task: Create OTA Progress UI
    - [ ] Add `DisplayManager::drawUpdateScreen(int progress, const char* status)` to show a visual progress bar.
    - [ ] Hook into OTA callbacks to update the LCD in real-time.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Local Web Update & Progress UI' (Protocol in workflow.md)

## Phase 2: GitHub & MQTT Integration
- [ ] Task: Implement GitHub Version Check
    - [ ] Create a service in `SideEyeNetworkManager` to query the GitHub Tags/Releases API using `HTTPClient`.
    - [ ] Compare the local `FIRMWARE_VERSION` with the latest remote tag.
- [ ] Task: Implement MQTT Triggered Update
    - [ ] Add a handler for the `ota_url` topic in the MQTT callback.
    - [ ] Use `httpUpdate.update(client, url)` to perform the flash from a remote URL.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: GitHub & MQTT Integration' (Protocol in workflow.md)

## Phase 3: Safety & Persistence
- [ ] Task: Verify Settings Retention
    - [ ] Perform end-to-end tests to ensure `LittleFS` is not wiped during the update process.
    - [ ] Add a "Post-Update" flag to show a "Update Successful" screen on the first boot after a flash.
- [ ] Task: Final Refinement
    - [ ] Add error handling for low-memory or failed downloads.
    - [ ] Ensure the device correctly handles "Waiting for Host" state if the host agent is active during an update.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Safety & Persistence' (Protocol in workflow.md)
