# Specification: Firmware Version Tracking & Reporting

## Context
Currently, the firmware has no defined version. The host software has no way to know which version of the firmware is running on the device.

## Goals
1.  **Version Definition:** Define a version string in the firmware.
2.  **Version Reporting:** Send the version to the host upon connection or request.
3.  **UI Display:** Optionally show the version on the device UI (e.g., in a "Boot" or "Info" screen).

## Requirements

### Firmware (C++)
-   **Version Constant:** Define a `VERSION` constant (e.g., `0.1.0`).
-   **Protocol Update:** Include the version in a response to the host.
-   **Serial Request:** Respond to a specific serial command (e.g., `{"type": "GetVersion"}`) with the version.

### Host (Rust)
-   The host should eventually be updated to log the firmware version.

## Non-Functional Requirements
-   **Consistency:** Use Semantic Versioning (SemVer).
