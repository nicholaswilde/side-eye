# Implementation Plan - MQTT Remote Settings Control

This plan outlines the steps to implement remote configuration of device settings via MQTT, including persistence and visual feedback.

## Phase 1: MQTT Command Infrastructure
- [x] Task: Expand MQTT subscription logic (7121)
    - [x] Update `SideEyeNetworkManager::reconnect()` to subscribe to the base set topic `side-eye/DEVICE_ID/set/#`.
    - [x] Update the MQTT callback to parse the sub-topic name to identify which setting is being targeted.
- [x] Task: Implement setting handlers (7121)
    - [x] Create a routing mechanism in `SideEyeNetworkManager` to map topics (e.g., `brightness`, `rotation`) to internal state updates.
    - [x] Add validation logic for each parameter (e.g., range checks for 0-100% or 0-255 brightness).
- [x] Task: Implement state feedback (7121)
    - [x] Add logic to publish updated values to corresponding `side-eye/DEVICE_ID/state/...` topics after a change.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: MQTT Command Infrastructure' (Protocol in workflow.md)

## Phase 2: Persistence & Application
- [x] Task: Update Configuration Management (7121)
    - [x] Update the `config.json` schema in LittleFS to include the new configurable parameters.
    - [x] Update `SideEyeNetworkManager` to save the updated JSON to LittleFS whenever a setting is changed.
- [x] Task: Apply settings to Hardware (7121)
    - [x] Ensure `DisplayManager` methods (like `setBacklight` and `setRotation`) are called immediately when MQTT commands are processed.
    - [x] Ensure `InputHandler` and `main.cpp` use the dynamic `cycle_duration` and alert thresholds.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Persistence & Application' (Protocol in workflow.md)

## Phase 3: UI Feedback & Refinement
- [x] Task: Implement visual confirmation (7121)
    - [x] Add `DisplayManager::showNotification(const char* message)` to briefly overlay a message on the LCD.
    - [x] Trigger this notification when a setting is successfully updated via MQTT.
- [x] Task: Final refinement and testing (7121)
    - [x] Verify that settings survive a hard reset (power cycle).
    - [x] Ensure no conflicts with the physical button overrides (e.g., if a user manually rotates, does MQTT state stay in sync?).
- [ ] Task: Conductor - User Manual Verification 'Phase 3: UI Feedback & Refinement' (Protocol in workflow.md)
