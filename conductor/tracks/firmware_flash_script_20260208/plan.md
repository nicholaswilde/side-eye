# Implementation Plan - Firmware Flash Script

This plan outlines the steps to create a Bash script for downloading and flashing SideEye firmware binaries to the ESP32-C6.

## Phase 1: Preparation & Scaffolding
- [x] Task: Create script file and define basic structure (81e7e83)
    - [ ] Create `firmware/scripts/flash.sh`
    - [ ] Add shebang and basic script headers
    - [ ] Define color constants and `log` function
- [x] Task: Implement dependency verification (25517f8)
    - [ ] Create `check_dependencies` function to verify `curl`, `grep`, `unzip`, and `esptool`
- [x] Task: Implement argument parsing (79f3cb8)
    - [ ] Parse version tag (optional first argument)
    - [ ] Parse `--dry-run` flag
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Preparation & Scaffolding' (Protocol in workflow.md)

## Phase 2: Firmware Retrieval & Environment [checkpoint: 1684c5c]
- [x] Task: Implement GitHub Release discovery (2f210ce)
    - [ ] Implement logic to fetch latest tag if no version is provided
    - [ ] Construct download URL for the firmware zip artifact
- [x] Task: Implement download and extraction (2a8135f)
    - [x] Create temporary directory (`mktemp -d`)
    - [x] Download firmware zip using `curl`
    - [x] Extract `bootloader.bin`, `partitions.bin`, and `firmware.bin`
- [x] Task: Implement Cleanup Logic (2a8135f)
    - [x] Ensure the temporary directory is removed on exit (using `trap`)
- [x] Task: Conductor - User Manual Verification 'Phase 2: Firmware Retrieval & Environment' (Protocol in workflow.md) (1684c5c)

## Phase 3: Hardware Interface & Flashing
- [ ] Task: Implement Port Auto-detection
    - [ ] Search for active `/dev/ttyACM*` and `/dev/ttyUSB*` ports
    - [ ] Implement fallback or selection if multiple ports are found
- [ ] Task: Implement Flashing Logic
    - [ ] Configure `esptool` command for `esp32c6`
    - [ ] Set flash addresses: `0x0000` (bootloader), `0x8000` (partitions), `0x10000` (firmware)
    - [ ] Integrate `--dry-run` logic to bypass the actual `esptool` call
- [ ] Task: Final script refinement
    - [ ] Ensure robust error messages and exit codes
    - [ ] Make the script executable (`chmod +x`)
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Hardware Interface & Flashing' (Protocol in workflow.md)
