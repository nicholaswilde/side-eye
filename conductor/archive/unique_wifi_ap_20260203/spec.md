# Specification: Unique Wi-Fi AP Name

## Context
Currently, the SideEye device uses a hardcoded Wi-Fi Access Point (AP) name, "SideEye-Setup", when entering configuration mode via `WiFiManager`. If multiple SideEye devices are in setup mode simultaneously, it becomes difficult for users to distinguish between them.

## Goals
1.  **Unique Identification:** Make the Wi-Fi AP name unique for each device by appending a portion of its hardware MAC address.
2.  **Consistent Branding:** Simplify the base AP name to "SideEye" while maintaining the unique suffix.
3.  **Visual Clarity:** Update all user-facing interfaces (LCD and Serial logs) to show the unique AP name.

## Requirements

### Firmware (C++)
-   **Retrieve MAC Address:** Fetch the ESP32 Base MAC address at startup.
-   **Generate Unique ID:** Extract the last 6 characters (3 bytes) of the MAC address and format them as an uppercase hexadecimal string.
-   **Unique AP Name:** Construct the AP name using the format `SideEye-XXXXXX` (e.g., `SideEye-A1B2C3`).
-   **Update WiFiManager:** Pass the unique AP name to `wm.autoConnect()`.
-   **Update UI:** Modify `configModeCallback` to display the unique AP name on the LCD.
-   **Serial Logging:** Log the unique AP name to the Serial console during the boot/setup process.

### Host (Rust)
-   Ensure that any future discovery logic or logging account for the unique name (though primary communication remains over USB Serial).

## Acceptance Criteria
-   When the device enters setup mode, the Wi-Fi SSID broadcast matches `SideEye-XXXXXX`.
-   The LCD screen clearly displays the unique SSID.
-   The Serial monitor shows the same unique SSID.
-   The unique ID is derived from the last 6 characters of the Base MAC address.

## Out of Scope
-   Changing the password for the AP (it will remain open/no password as per current `WiFiManager` usage).
-   Implementing custom captive portal HTML changes.
