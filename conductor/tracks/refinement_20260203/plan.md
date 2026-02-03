# Implementation Plan: Refinement - Core Streaming, UI Polish, and Wi-Fi Integration

## Phase 1: Host Application Refinement
- [x] Task: Host - Refine Serial Auto-detection logic in `main.rs`. 8e28bcf
    - [x] Update `autodetect_port` to handle potential PID variations or provide better error messages.
    - [x] Implement `clap` for CLI argument parsing.
    - [x] Add support for `--version`, `--help`, `--verbose`, and `--dry-run`.
- [x] Task: Host - Implement Robust Reconnection Loop. 8e28bcf
    - [x] Refactor the main loop to handle `serialport` errors (e.g., device unplugged) by sleeping and retrying `autodetect_port`.
- [x] Task: Host - Verify and Test. 8e28bcf
    - [x] Run `cargo run` and physically unplug/replug the device to ensure it recovers.

## Phase 2: Firmware Setup & Wi-Fi Integration [checkpoint: 0d96a53]
- [x] Task: Firmware - Update `platformio.ini` dependencies. 85e7a89
    - [x] Add `tzapu/WiFiManager` to `lib_deps`.
- [x] Task: Firmware - Implement WiFiManager Logic in `main.cpp`. 92285da
    - [x] Include `<WiFiManager.h>`.
    - [x] In `setup()`, initialize `WiFiManager` and handle the auto-connect / captive portal flow.
    - [x] Display connection status on the LCD during boot.
- [x] Task: Firmware - Verify Wi-Fi Connection. 17a7a61
    - [x] Flash firmware, connect to "SideEye-Setup" AP, configure Wi-Fi, and verify it reconnects on reboot.

## Phase 3: Display UI Polish [checkpoint: 6192d36]
- [x] Task: Firmware - Refine `updateDisplay` function. 0885b90
    - [x] Implement the layout defined in the Spec (Header, Labels, Colors).
    - [x] Add a "Waiting for Host..." screen that persists until the first valid serial packet is received.
    - [x] Add a small footer indicator for the ESP32's own Wi-Fi status.
- [x] Task: Firmware - Optimize String Handling. 0885b90
    - [x] Ensure the parsing logic is safe against buffer overflows or partial packets.

## Phase 4: Integration & Documentation [checkpoint: b4e6e08]
- [x] Task: Integration - End-to-End Test. eb7c595
    - [x] Run the refined Host with the new Firmware. Verify data syncs correctly and looks good.
- [x] Task: Documentation - Update README. 012b487
    - [x] Update `README.md` with new build instructions (Wi-Fi setup steps).
- [x] Task: Conductor - User Manual Verification 'Integration & Documentation' (Protocol in workflow.md) ea81e06
