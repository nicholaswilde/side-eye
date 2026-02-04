# Implementation Plan: Firmware Modularization

## Phase 1: Core Managers
- [x] Task: Firmware - Create `DisplayManager` and migrate drawing functions.
- [x] Task: Firmware - Create `InputHandler` and migrate button logic.
- [x] Task: Conductor - User Manual Verification 'Core Managers' (Confirmed by build)

## Phase 2: Connectivity & Storage
- [x] Task: Firmware - Create `NetworkManager` and migrate Wi-Fi/MQTT logic.
- [x] Task: Firmware - Create `SyncManager` and migrate SD card logic.
- [x] Task: Conductor - User Manual Verification 'Connectivity' (Confirmed by build)

## Phase 3: Final Orchestration
- [x] Task: Firmware - Refactor `main.cpp` to use the new manager instances.
- [x] Task: Firmware - Verify all features (Rotation, Paging, Alerts, Sync) work as before.
- [x] Task: Conductor - User Manual Verification 'Final System' (Confirmed by build)
