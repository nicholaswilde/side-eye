# Implementation Plan: Firmware Modularization

## Phase 1: Core Managers
- [ ] Task: Firmware - Create `DisplayManager` and migrate drawing functions.
- [ ] Task: Firmware - Create `InputHandler` and migrate button logic.
- [ ] Task: Conductor - User Manual Verification 'Core Managers' (Protocol in workflow.md)

## Phase 2: Connectivity & Storage
- [ ] Task: Firmware - Create `NetworkManager` and migrate Wi-Fi/MQTT logic.
- [ ] Task: Firmware - Create `SyncManager` and migrate SD card logic.
- [ ] Task: Conductor - User Manual Verification 'Connectivity' (Protocol in workflow.md)

## Phase 3: Final Orchestration
- [ ] Task: Firmware - Refactor `main.cpp` to use the new manager instances.
- [ ] Task: Firmware - Verify all features (Rotation, Paging, Alerts, Sync) work as before.
- [ ] Task: Conductor - User Manual Verification 'Final System' (Protocol in workflow.md)
