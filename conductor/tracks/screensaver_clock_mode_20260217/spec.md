# Specification - Screensaver / Clock Mode

## Overview
Transform the SideEye into a functional desk clock when the system is idle. Instead of simply turning off the display, the device will transition to a high-contrast "Screensaver" mode featuring a large digital clock and date, driven by the host system's time.

## Functional Requirements
- **Host Time Sync:**
  - The Rust host agent shall include the current system time (HH:MM:SS) and date (YYYY-MM-DD) in the 1-second telemetry JSON stream.
- **Trigger Logic:**
  - **Host-Initiated:** The host agent sends an `is_idle` boolean flag (true when screen is locked or idle).
  - **Local Timeout:** If no serial data is received for 1 minute, the device automatically enters Clock Mode.
- **Clock UI:**
  - A dedicated "Clock Page" using a large, bold font for the time.
  - The date displayed in a smaller font below or above the time.
  - Status icons (WiFi, MQTT, etc.) remain visible in the footer.
  - The background can follow the active theme (e.g., `background.jpg` or solid color).
- **Wake Logic:**
  - Any physical button press or the host setting `is_idle` to `false` returns the device to the last active dashboard page.
- **Power Management:**
  - Provide a configuration option to either keep the clock on indefinitely (optionally dimmed) or turn off the backlight after a further delay (e.g., 5 minutes of clock mode).

## Acceptance Criteria
- [ ] The device switches to the Large Digital Clock UI when the host system is locked.
- [ ] The time on the device is synchronized with the host system time within 1 second.
- [ ] Interacting with the PC (waking it up) immediately restores the SideEye telemetry dashboard.
- [ ] If the host agent is stopped, the clock appears after 60 seconds of silence.

## Out of Scope
- Configurable clock faces (analog, binary) — limited to Digital for this track.
- Setting alarms or timers directly on the device.
