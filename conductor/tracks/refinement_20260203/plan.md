# Implementation Plan: Refinement - Core Streaming, UI Polish, and Wi-Fi Integration

## Phase 1: Host Application Refinement
- [x] Task: Host - Refine Serial Auto-detection logic in `main.rs`. 8e28bcf
    - [x] Update `autodetect_port` to handle potential PID variations or provide better error messages.
    - [x] Implement `clap` for CLI argument parsing.
    - [x] Add support for `--version`, `--help`, `--verbose`, and `--dry-run`.
- [x] Task: Host - Implement Robust Reconnection Loop. 8e28bcf
    - [x] Refactor the main loop to handle `serialport` errors (e.g., device unplugged) by sleeping and retrying `autodetect_port`.
- [~] Task: Host - Verify and Test.
    - [ ] Run `cargo run` and physically unplug/replug the device to ensure it recovers.

## Phase 2: Firmware Setup & Wi-Fi Integration
- [ ] Task: Firmware - Update `platformio.ini` dependencies.
    - [ ] Add `tzapu/WiFiManager` to `lib_deps`.
- [ ] Task: Firmware - Implement WiFiManager Logic in `main.cpp`.
    - [ ] Include `<WiFiManager.h>`.
    - [ ] In `setup()`, initialize `WiFiManager` and handle the auto-connect / captive portal flow.
    - [ ] Display connection status on the LCD during boot.
- [ ] Task: Firmware - Verify Wi-Fi Connection.
    - [ ] Flash firmware, connect to "SideEye-Setup" AP, configure Wi-Fi, and verify it reconnects on reboot.

## Phase 3: Display UI Polish
- [ ] Task: Firmware - Refine `updateDisplay` function.
    - [ ] Implement the layout defined in the Spec (Header, Labels, Colors).
    - [ ] Add a "Waiting for Host..." screen that persists until the first valid serial packet is received.
    - [ ] Add a small footer indicator for the ESP32's own Wi-Fi status.
- [ ] Task: Firmware - Optimize String Handling.
    - [ ] Ensure the parsing logic is safe against buffer overflows or partial packets.

## Phase 4: Integration & Documentation
- [ ] Task: Integration - End-to-End Test.
    - [ ] Run the refined Host with the new Firmware. Verify data syncs correctly and looks good.
- [ ] Task: Documentation - Update README.
    - [ ] Update `README.md` with new build instructions (Wi-Fi setup steps).
- [ ] Task: Conductor - User Manual Verification 'Integration & Documentation' (Protocol in workflow.md)
