# Specification - OTA Firmware Updates

## Overview
Enable the SideEye device to update its firmware over the network (Wi-Fi), reducing reliance on physical USB connections for maintenance. This includes automatic updates from GitHub and manual triggers via Web and MQTT.

## Functional Requirements
- **Update Mechanisms:**
  - **GitHub Auto-Update:** The device shall check GitHub Releases for newer versions and download the `firmware.bin` if an update is found.
  - **Web Update:** A minimal web interface at `/update` shall allow users to upload a firmware binary from a browser.
  - **MQTT Update:** The device shall listen to `side-eye/DEVICE_ID/set/ota_url` to trigger a download from a specific location.
- **User Interface & Feedback:**
  - **Visual:** A dedicated update screen shall show a progress bar and status text.
  - **Remote:** MQTT state topic `side-eye/DEVICE_ID/state/ota_progress` shall report percentage and status.
- **Security & Safety:**
  - The device shall verify the integrity of the update before applying (ESP32 built-in check).
  - Persistence of `config.json` shall be maintained across updates.

## Acceptance Criteria
- [ ] Navigating to `<IP>/update` and uploading a valid `.bin` successfully updates the device.
- [ ] Sending a GitHub release URL via MQTT triggers a download and flash.
- [ ] Progress (0-100%) is visible on the LCD during the process.
- [ ] The device reboots into the new firmware and retains previous MQTT settings.
