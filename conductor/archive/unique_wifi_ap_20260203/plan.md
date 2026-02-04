# Implementation Plan: Unique Wi-Fi AP Name

## Phase 1: Core Logic & Identification
- [x] Task: Firmware - Implement MAC-based Unique ID generation.
    - [x] Create a helper function to retrieve the Base MAC address.
    - [x] Format the last 6 characters as an uppercase hex string.
- [x] Task: Firmware - Integrate Unique ID into serial logging.
    - [x] Log the generated AP name to Serial at the start of `setup()`.
- [x] Task: Conductor - User Manual Verification 'Core Logic' (Protocol in workflow.md)

## Phase 2: UI & Connectivity Integration
- [x] Task: Firmware - Update setup screen and config mode UI.
    - [x] Modify `configModeCallback` to display the dynamic `SideEye-XXXXXX` name.
    - [x] Ensure the "BOOTING..." or setup screens reflect the unique ID if necessary.
- [x] Task: Firmware - Update WiFiManager auto-connect logic.
    - [x] Pass the dynamically generated AP name to `wm.autoConnect()`.
- [x] Task: Conductor - User Manual Verification 'UI & Connectivity' (Protocol in workflow.md)
