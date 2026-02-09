# Specification - Firmware Flash Script

## Overview
Create a Bash-based flash script (`flash.sh`) for SideEye that automates the process of downloading firmware binaries from GitHub releases and flashing them to the ESP32-C6 hardware. This script improves the user experience for non-developers who want to update their device without setting up a full development environment.

## Functional Requirements
- **Target Environment:** Linux (Bash).
- **Firmware Retrieval:**
  - Support downloading the latest release automatically.
  - Support downloading a specific version tag provided as a command-line argument.
- **Hardware Support:** Strictly locked to the ESP32-C6 chip.
- **Flashing Logic:**
  - Flash three binary files: `bootloader.bin`, `partitions.bin`, and `firmware.bin`.
  - Use hardcoded standard flash addresses for these files.
- **User Experience:**
  - **Port Auto-detection:** Automatically identify the SideEye serial port (searching `/dev/ttyACM*` and `/dev/ttyUSB*`).
  - **Dry-run Mode:** Provide a `--dry-run` flag to simulate the process without executing `esptool`.
  - **Cleanup:** Automatically delete temporary files and directories used during the download and extraction process.
- **Dependencies:** Check for and notify the user if required tools (`curl`, `grep`, `unzip`, `esptool`) are missing.

## Non-Functional Requirements
- **Simplicity:** The script should be easy to run with a single command.
- **Robustness:** Include error handling for network failures, missing devices, and failed flashing attempts.

## Acceptance Criteria
- [ ] Running `./flash.sh` without arguments detects the device, downloads the latest firmware, and flashes it.
- [ ] Running `./flash.sh vX.Y.Z` downloads and flashes the specified version.
- [ ] Running `./flash.sh --dry-run` logs the intended actions without flashing.
- [ ] Temporary files are removed after successful or failed execution.
- [ ] The script correctly identifies the ESP32-C6 chip during the `esptool` command.

## Out of Scope
- Support for Windows or macOS (initial release focuses on Linux).
- Support for other ESP32 chip variants.
- In-place compilation of firmware (binary-only flashing).
