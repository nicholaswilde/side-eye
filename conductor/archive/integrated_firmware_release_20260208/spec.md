# Specification: Integrated Firmware and Host Release

## Overview
This track integrates the ESP32 firmware build and release process into the existing Rust host release workflow. The goal is to provide a single, unified release entry on GitHub containing both the host binaries (bins, .deb, .rpm) and a zipped bundle of the firmware artifacts (`firmware.bin`, `bootloader.bin`, `partitions.bin`).

## Functional Requirements
1.  **Unified Release Workflow:**
    -   Merge logic from the external `frame-fi` CI workflow into the existing `side-eye` `release.yml`.
    -   The firmware build should be a prerequisite or a parallel job to the host artifact upload.
2.  **Firmware Build & Packaging:**
    -   Build the ESP32-C6 firmware using PlatformIO.
    -   Capture the three essential binaries: `firmware.bin`, `bootloader.bin`, and `partitions.bin`.
    -   Bundle these files into a single zip archive named `side-eye-<tag_name>-firmware.zip`.
3.  **Release Integration:**
    -   Pass the release version tag (e.g., `v1.2.3`) to the firmware build process for internal version alignment.
    -   Upload the firmware zip file as an asset to the same GitHub Release created for the host.
4.  **Preservation of Existing Features:**
    -   Retain all existing host build targets (AMD64, ARM64, etc.) and package formats (.deb, .rpm).
    -   Maintain the "Draft Release" publication mode.

## Non-Functional Requirements
-   **Reliability:** The firmware build must succeed for the entire release process to be considered successful.
-   **Efficiency:** Reuse caching for PlatformIO dependencies where possible to speed up the integrated workflow.

## Acceptance Criteria
- [ ] A single GitHub release is created (in Draft state) when a new version tag is pushed.
- [ ] The release contains host binaries/packages for all supported architectures.
- [ ] The release contains a zip file named `side-eye-<tag_name>-firmware.zip`.
- [ ] The firmware zip file contains `firmware.bin`, `bootloader.bin`, and `partitions.bin`.
- [ ] The firmware version (if reported by the device) matches the release tag.

## Out of Scope
-   Automated firmware updates (OTA) via the host application.
-   Multi-firmware flavor releases (e.g., different screen types) unless specified in the project's default build.
