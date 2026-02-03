# Implementation Plan: Unique Wi-Fi AP Name

## Phase 1: Core Logic & Identification
- [ ] Task: Firmware - Implement MAC-based Unique ID generation.
    - [ ] Create a helper function to retrieve the Base MAC address.
    - [ ] Format the last 6 characters as an uppercase hex string.
- [ ] Task: Firmware - Integrate Unique ID into serial logging.
    - [ ] Log the generated AP name to Serial at the start of `setup()`.
- [ ] Task: Conductor - User Manual Verification 'Core Logic' (Protocol in workflow.md)

## Phase 2: UI & Connectivity Integration
- [ ] Task: Firmware - Update setup screen and config mode UI.
    - [ ] Modify `configModeCallback` to display the dynamic `SideEye-XXXXXX` name.
    - [ ] Ensure the "BOOTING..." or setup screens reflect the unique ID if necessary.
- [ ] Task: Firmware - Update WiFiManager auto-connect logic.
    - [ ] Pass the dynamically generated AP name to `wm.autoConnect()`.
- [ ] Task: Conductor - User Manual Verification 'UI & Connectivity' (Protocol in workflow.md)
