# Specification: Disconnected SD Card Access

## Overview
Currently, when the SideEye firmware is not connected to the host agent, the display is locked to a "Waiting..." screen. This track enables the user to navigate to and view the SD Card information page even while disconnected, allowing local storage status checks (capacity and usage) at any time.

## Functional Requirements
1.  **Enable Navigation While Disconnected**: The physical button must allow cycling to the SD Card page even when `state.connected` is false.
2.  **Local SD Telemetry**: The SD Card page must calculate and display Total, Used, and Free space using local ESP32 SD library calls, ensuring the data is accurate without host input.
3.  **Disconnected Sync Status**: On the SD Card page, the "Sync" status label should display "Offline" or "Disconnected" when the host is not connected, instead of "Idle".
4.  **UI Consistency**: The "Waiting..." status indicator must remain visible (e.g., in the status line) while on other pages in a disconnected state to remind the user that system telemetry is unavailable.

## Non-Functional Requirements
- **Performance**: SD space calculations must not block the main loop or interfere with button responsiveness.

## Acceptance Criteria
- [ ] Pressing the button while "Waiting..." cycles to the SD Card page.
- [ ] SD Card page shows correct Total/Used MB based on the physical card inserted.
- [ ] Sync status reflects "Disconnected" when the host agent is not running.
- [ ] The device can return to the "Waiting..." screen via button cycle.

## Out of Scope
- Displaying historical network telemetry while disconnected.
- Performing actual file synchronization while disconnected.
- Accessing Host-specific identity info (Hostname, IP, etc.) beyond last known state.
