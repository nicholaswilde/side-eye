# Specification - Global Status Indicators

## Overview
Enhance the "at-a-glance" utility of the SideEye by adding a dedicated status bar in the UI footer. This bar will display small pixel-art icons representing the connectivity and operational state of key subsystems (MQTT, Host, SD, BLE) across all dashboard pages.

## Functional Requirements
- **Icon Set:** Implement a series of minimalist pixel-art icons (approx. 8x8 or 10x10 pixels):
  - **WiFi:** Current circle replaced or augmented by a signal icon.
  - **MQTT:** A plug or cloud icon (Green = Connected, Red/Grey = Disconnected).
  - **Host:** A computer or link icon (Green = Streaming, Orange = Waiting).
  - **SD Card:** A card icon (Green = Mounted, Red = Error, Blue = Syncing).
  - **BLE:** A Bluetooth icon (Blue = Scanning, Green = User Present).
- **Positioning:** The icons shall be arranged in a horizontal row in the bottom-left of the display, opposite the version number.
- **Global Visibility:** The status footer shall be rendered by `DisplayManager` on every dashboard page during the dynamic update cycle.
- **Dynamic Updates:** Icons must update their color/state immediately when the underlying `SystemState` changes.

## Acceptance Criteria
- [ ] Every page (Identity, Resources, etc.) displays the status icon row in the footer.
- [ ] The MQTT icon correctly reflects the connection state to the broker.
- [ ] The SD icon changes color/style when a sync operation is active.
- [ ] Proximity detection via BLE is visually confirmed by the Bluetooth icon state.
- [ ] The icons do not overlap with page-specific data or the version string.

## Out of Scope
- Detailed logs or pop-up messages for status changes (handled by existing banner/notifications).
- Interactive touch or button selection of icons.
