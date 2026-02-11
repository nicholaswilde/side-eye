# Specification - MQTT Remote Settings Control

## Overview
Enable users to remotely configure device settings on the SideEye ESP32 hardware using MQTT. This feature allows for dynamic adjustments to the display, alerts, and system behavior without requiring a re-flash or local interaction, integrating seamlessly with smart home platforms like Home Assistant.

## Functional Requirements
- **MQTT Command Interface:**
  - The device shall subscribe to individual "set" topics for various parameters.
  - Supported parameters:
    - `brightness`: 0-255 (PWM)
    - `rotation`: 1 or 3 (Landscape/Inverted Landscape)
    - `cycle_duration`: Time in milliseconds for page rotation.
    - `cpu_warning`, `cpu_critical`, `ram_warning`, `ram_critical`: Percentages (0-100).
    - `mqtt_interval`: Telemetry update frequency in milliseconds.
    - `discovery_prefix`: Home Assistant MQTT discovery prefix.
- **Feedback & Confirmation:**
  - **MQTT State:** The device shall publish the updated value to a corresponding `state` topic upon successful change.
  - **Visual:** The device LCD shall briefly display a "Settings Updated" notification or a similar visual cue.
  - **Serial:** All received setting changes shall be logged to the serial console.
- **Persistence:**
  - All settings changed via MQTT shall be saved to the `config.json` file in LittleFS.
  - The device shall load these settings from LittleFS during the `setup()` sequence.

## Non-Functional Requirements
- **Responsiveness:** Setting changes should be applied within 500ms of receiving the MQTT message (excluding the time taken for visual fade-ins if applicable).
- **Robustness:** Invalid values (e.g., brightness > 255) shall be rejected, and an error message should be logged to serial.

## Acceptance Criteria
- [ ] Publishing `200` to `side-eye/DEVICE_ID/set/brightness` changes the LCD brightness and saves the value.
- [ ] The device publishes the new brightness value back to `side-eye/DEVICE_ID/state/brightness`.
- [ ] After a power cycle, the device resumes with the last configured brightness and rotation.
- [ ] A visual confirmation appears on the screen when a setting is changed.
- [ ] Invalid data types or out-of-range values do not crash the device.

## Out of Scope
- Changing Wi-Fi credentials via MQTT (this remains handled by `WiFiManager` for security).
- OTA (Over-the-Air) firmware updates via MQTT (handled by the flash script/GitHub releases).
